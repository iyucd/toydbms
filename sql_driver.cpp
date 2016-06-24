#include <cctype>
#include <fstream>
#include <cassert>
#include <iostream>
#include <string> 
#include <vector>
#include "btree.h" // btree if from https://panthema.net/2007/stx-btree/ 
#include "Table.h"
#include "sql_driver.h"
#include "sql_scanner.h"
#include "globals.h"
#include "joinfuncs.h"

using namespace std;

extern void createTableEntry(UCD::SQLExpression *ep);
extern string define_table(string table_name, unsigned long num_columns, vector<string> &column_names, 
    vector<bool> primary_keys, vector<bool> keys, vector<stx::btree<string, int> > indexes);
extern void createRowEntry(UCD::SQLExpression *ep);
extern string insert_row(string table_name, unsigned long num_values, vector<string> &values);
extern void createSFWEntry(UCD::SQLExpression *ep);
extern void parseSelectNWhere();
extern string groupBy();
extern string exec_SFW();
extern bool reading_select;
extern Table new_table;
extern newRow new_row;
extern newSFW new_sfw;
extern UCD::SQLExpression* rcompile(UCD::SQLExpression *e);
extern bool isIndexSelectCandidate();
extern string indexSelection();
extern void init_new_sfw();
extern string exec_in_SFW();

// Reverses a vector
template <typename T>
vector<T> reverse_vector(vector<T> vec) {
    vector<T> temp;
    for (int i = vec.size()-1; i >= 0; i--)
        temp.push_back(vec[i]);
    return temp;
}

/* yyparse wants integers and CSIF machines have larger pointers than ints */
/* So need conversion routines between ints and node pointers **************/
UCD::SQLExpression ebuf[MAXNODE];

UCD::SQLDriver::~SQLDriver() {
    delete(scanner);
    scanner = nullptr;
    delete(parser);
    parser = nullptr;
}

void UCD::SQLDriver::parse( const char * const filename )  {
    std::cout << "on parse method\n";
    assert( filename != nullptr );
    std::ifstream in_file( filename );
    if( ! in_file.good() ) {
        exit( EXIT_FAILURE );
    }
    parse_helper( in_file );
    return;
}

void UCD::SQLDriver::parse( std::istream &stream )  {
    if( ! stream.good()  && stream.eof() ) {
        return;
    }
    //else
    parse_helper( stream );
    return;
}

void UCD::SQLDriver::parse_helper( std::istream &stream )  {
    //std::cout << "on helper function\n";
    delete(scanner);
    try {
        scanner = new UCD::SQLScanner( &stream );
    } catch( std::bad_alloc &ba ) {
        std::cerr << "Failed to allocate scanner: (" << ba.what() << "), exiting!!\n";
        exit( EXIT_FAILURE );
    }
    delete(parser);
    try {
        parser = new UCD::SQLParser( (*scanner) /* scanner */, (*this) /* driver */ );
    } catch( std::bad_alloc &ba ) {
        std::cerr << "Failed to allocate parser: (" <<
        ba.what() << "), exiting!!\n";
        exit( EXIT_FAILURE );
    }
    const int accept( 0 );
    if (parser->parse() != accept) {
        //std::cerr << "Parse failed!!\n";
        this->quitted = true;
    }
    //std::cout << "Done parsing\n";
    return;
}

void UCD::SQLDriver::print( ) {
    for(int i=0;i<freen;i++) {
        ebuf[i].print();
    }
}

void UCD::SQLDriver::clear( ) {
    for(int i=0;i<freen;i++) {
        ebuf[i].clear();
    }
    this->freen = 0;
}

bool UCD::SQLDriver::quit() {
    return this->quitted;
}

UCD::SQLExpression* UCD::SQLDriver::makearray(UCD::SQLExpression e){
    UCD::SQLExpression *epp= nullptr;
    UCD::SQLExpression ep = e;
    int i, size = listlen(ep);

    if(size==0) return nullptr;
    if(size < 0) {
        std::cout <<"::Bad list structure\n";
        exit(0);
    }
    epp = new UCD::SQLExpression();
    for(i=size-1; i>=0; i--) {
        if(&ep != nullptr || ep.getFunc() != OP_RLIST) {
            std::cout <<"::Not a list element\n";
            return 0;
        }
        epp[i] = *ep.getExpression(0);
        ep = *ep.getExpression(1);
    }
    return epp;
}

int UCD::SQLDriver::listlen(UCD::SQLExpression e){
    UCD::SQLExpression ep = e;
    int i =0;

    while(&ep != nullptr) {
        if(ep.getFunc() != OP_RLIST)
            return -1;		/* Not a list element */
        ep = *ep.getExpression(1);
        i++;
    }
    return i;
}

UCD::SQLExpression* UCD::SQLDriver::cvt_itoe(int i) {
    UCD::SQLExpression* e = 0;

    //std::cout << "::cvt_itoe called with " << i << std::endl;
    if (!i) {
        //std::cout << "will return null\n";
        return e;
    }
    if (i < 0) {
        std::cout << i << ": Messed up index - too low\n";
        return e;
    }
    if (i > MAXNODE) {
        std::cout << i << ": Messed up index - too high\n";
        return e;
    }
    e = &ebuf[i];
    return e;
}

int UCD::SQLDriver::makeexpr(int op, int cnt, int arg1, int arg2) {
    UCD::SQLExpression* ep;
    //int i, size;

    //std::cout << ":make_expr called with " << op << " " << cnt << " " << arg1 << " " << arg2 << std::endl;
    /* yyparse wants integers not pointers, and on CSIF machines they are incompatible */
    /* So allocate from an array, and return a modified index */
    if(freen < MAXNODE) {
        freen++;
        ep = &ebuf[freen];
        ep->index = freen;
    } else {
        std::cout << "Out of expression nodes\n";
        return 0;
    }
    ep->setFunc(op);
    ep->setCount(cnt);
    ep->setExpression(0, cvt_itoe(arg1));
    switch(ep->getFunc()) {
        default:
            ep->setExpression(1, cvt_itoe(arg2));
            break;
        case OP_COLUMNDEF:
            ep->setNum(1, arg2);
            break;
    }
    //std::cout << "::returning " << ep->index << std::endl;
    return freen;
}

int UCD::SQLDriver::makestrexpr(std::string *str) {
    int i = makeexpr(OP_STRING,1,0,0);
    ebuf[i].setCount(1);
    ebuf[i].setData(0, *str);
    //std::cout << "mystr: " << str << std::endl;
    //std::cout << "data: " << ebuf[i].getData(0) << std::endl;
    return i;
}

int UCD::SQLDriver::makename(int op, std::string *str) {
    return makename(op, str, nullptr);
}

int UCD::SQLDriver::makename(int op, std::string *str, std::string *alt_str) {
    int i = makeexpr(op,1,0,0);
    ebuf[i].setCount(1);
    ebuf[i].setName(0, *str);
    if(nullptr != alt_str) {
        //std::cout << "makename called with " << i << " " << op << " " << *str << " : " << *alt_str << std::endl;
        ebuf[i].setAltData(0, *alt_str);
    }
    //std::cout << "::makename: returning " << i << std::endl;
    return i;
}

int UCD::SQLDriver::setname(int op, std::string *str) {
    return setname(op, str, nullptr);
}

int UCD::SQLDriver::setname(int op, std::string *str, std::string *alt_str) {
    //std::cout << "Set name string\n";
    int ei = makename(OP_FNAME, str, alt_str);
    UCD::SQLExpression* ep;
//    std::cout << "::setname called with " << op << " " << ei << " " << *str << std::endl;
//    if(nullptr != alt_str)
//        std::cout<< " : " << *alt_str << std::endl;
    ep = cvt_itoe(ei);
    if(nullptr == ep) return 0;
    //std::cout <<"::Setname: name=" << ep->getName(0) << std::endl;
    ep->setFunc(op);
    return ei;
}

int UCD::SQLDriver::makenum(int v) {
    //std::cout << "Make number\n";
    int i = makeexpr(OP_NUMBER,1,0,0);
    ebuf[i].setCount(1);
    ebuf[i].setNum(0, v);
    return i;
}

UCD::SQLExpression* UCD::SQLDriver::compile(UCD::SQLExpression *e) {
    //std::cout << "Compile\n";
    UCD::SQLExpression* ep = rcompile(e);

    return ep;
}

UCD::SQLExpression* UCD::SQLDriver::optimize(UCD::SQLExpression *e){
    //std::cout << "Im an optimize\n";

    return e;
}

UCD::SQLExpression* UCD::SQLDriver::evalexpr(UCD::SQLExpression *ep) {
    //std::cout << "Eval expr\n";
    //this->print();    //Displays all the nodes used for this expression
    //print_exptree(ep, 0);
    //std::cout << std::endl;

    if (ep->getFunc() == OP_CREATETABLE) {
        new_table.init();
        createTableEntry(ep);
        // Reverse the column name order so it is correctly ordered. At this point it is reversed.
        new_table.reverse_column_names();
        // Reverse primary_key_ and key_ vectors so they match the column_name_ vector order.
        new_table.reverse_key_vectors();
        // Now place table in data file
        vector<string> col_names = new_table.GetColumnNames();
        string result = define_table(new_table.GetTableName(),new_table.GetColumnNamesVecSize(),
            col_names, new_table.GetPrimaryKeys(), new_table.GetKeys(),new_table.indexes);
        if (result != "0")
            cerr << "ERROR: " << result << endl;
    }
    else if (ep->getFunc() == OP_INSERTROW) {
        new_row.table_name.clear();
        new_row.values.clear();
        createRowEntry(ep);
        // Reverse the row order so it is correctly ordered. At this point it is reversed.
        new_row.values = reverse_vector(new_row.values);
        // Now insert the row to the data file
        string result = insert_row(new_row.table_name, new_row.values.size(), new_row.values);
        if (result != "0")
            cerr << "ERROR: " << result << endl;
    }
    else if (ep->getFunc() == OP_PROJECTION) {
        init_new_sfw();
        reading_select = false;
        createSFWEntry(ep);
        parseSelectNWhere();
        // Reverse order so it is the same order as inputed.
        new_sfw.proj_expressions = reverse_vector(new_sfw.proj_expressions);
        new_sfw.proj_as_column_names = reverse_vector(new_sfw.proj_as_column_names);
        new_sfw.in_list = reverse_vector(new_sfw.in_list
            );
        // check if join is required and choose appropriate algorithm
        checkForJoin();

        // Call appropriate function (particular join, or select)
        string result;
        
        // If reading a SFW which uses IN keyword
        if (new_sfw.isInSFW) {
            result = exec_in_SFW();
        } else if(new_sfw.callsGroupBy) {
			result = groupBy();
        } else if(new_sfw.isIndexJoin) {
            result = indexJoin();
        } else if(new_sfw.isHashJoin) {
            result = hashJoin();
        } else if(new_sfw.isNestedLoopJoin) {
            result = nestedLoopJoin();
        } else if(new_sfw.multipleTables) {
            result = crossProduct();
        } else if (isIndexSelectCandidate()) {
            result = indexSelection();
        } else {
            result = exec_SFW();
        }
        
        if (result != "0")
            cerr << "ERROR: " << result << endl;
    }
    return ep;    
}

void UCD::SQLDriver::print_relation(UCD::SQLExpression *e) {
    //std::cout << "Im an print_relation"<< std::endl;
}

void UCD::SQLDriver::print_exptree(UCD::SQLExpression *e, int lev) {
    //std::cout << lev << "]  To print\n";
    UCD::SQLExpression* ep = e;
    register int i, slev=lev;
    if(nullptr == ep) {
        std::cout << "() "; 
        return; 
    }
    //std::cout << ep->getFunc() << "] ";
    switch(ep->getFunc()) {
        /* Literals */
        case OP_NUMBER:	        std::cout << ep->getNum(0); return;
        case OP_STRING:	        std::cout << "{ " << ep->getData(0) << " }"; return;
        case OP_NULL:	        std::cout << "NULL "; return;
        /* Names */
        case OP_COLNAME:        std::cout << "COLUMN: " << ep->getName(0);
                                if(ep->getAltData(0).compare("") != 0)
                                    std::cout << "\t TABLE: " << ep->getAltData(0);
                                return;
        case OP_TABLENAME:      std::cout << "TABLE: " << ep->getName(0); return;
        case OP_FNAME:          std::cout << "FUNC: " << ep->getName(0); return;
        case OP_COLUMNDEF:
            std::cout << "(COLSPEC ";
            std::cout << (ep->getNum(1)==1 ? "KEY" : ep->getNum(1)==3 ? "PRIMARY" : "");
            print_exptree(ep->getExpression(0), lev + 2);
            std::cout << ")";
            return;
        /* Relational operators */
        case OP_PROJECTION:     std::cout << "(PROJECT "; break;
        case OP_PROJECTROW:     std::cout << "(PROJECTROW "; break;
        case OP_SELECTION:      std::cout << "(SELECT "; break;
        case OP_SELECTROW:      std::cout << "(SELECTROW "; break;
        case OP_PRODUCT:        std::cout << "(PRODUCT "; break;
        case OP_SORT:           std::cout << "(SORT "; break;
        case OP_GROUP:          std::cout << "(GROUP "; break;
        case OP_DELTA:          std::cout << "(DELTA "; break;
        case OP_CREATETABLE:    std::cout << "(CREATETABLE "; break;
        case OP_INSERTROW:      std::cout << "(INSERTROW "; break;
        case OP_UPDATE:         std::cout << "(UPDATE "; break;
        case OP_DROPTABLE:      std::cout << "(DROP "; break;
        case OP_DELETE:         std::cout << "(DELETE "; break;
        case OP_DELETEROW:      std::cout << "(DELETEROW "; break;
        case OP_DISTINCT:       std::cout << "(DISTINCT "; break;
        case OP_GETNEXT:        std::cout << "(GETNEXT "; break;
        case OP_ITERATE:        std::cout << "(ITERATE "; break;
        case OP_PLUS:	        std::cout << "(+ "; break;
        case OP_BMINUS:	        std::cout << "(- "; break;
        case OP_TIMES:	        std::cout << "(* "; break;
        case OP_DIVIDE: 	    std::cout << "(/ "; break;
        case OP_AND:	        std::cout << "(AND "; break;
        case OP_OR:	            std::cout << "(OR "; break;
        case OP_NOT:	        std::cout << "(! "; break;
        case OP_GT:	            std::cout << "(> "; break;
        case OP_LT:	            std::cout << "(< "; break;
        case OP_EQUAL:	        std::cout << "(== "; break;
        case OP_NOTEQ:	        std::cout << "(<> "; break;
        case OP_GEQ:	        std::cout << "(>= "; break;
        case OP_LEQ:	        std::cout << "(<= "; break;
        case OP_SORTSPEC:       std::cout << "(SORTSPEC "; break;
        case OP_OUTCOLNAME:     std::cout << "(AS "; break;
        case OP_IN: 	        std::cout << "(IN "; break;
        case OP_RLIST:	        std::cout << "(RLIST "; break;
        case OP_DESCRIBE:       std::cout << "(DESCRIBE "; break;
        case OP_SHOWTABLES:     std::cout << "(SHOWTABLES "; break;
        case OP_FCALL:          std::cout << "(FCALL "; break;
        default:	            std::cout << "(" << ep->getFunc(); break;
    }
    std::cout << std::endl;
    lev += 2;
    for(i=0; i<lev; i++) std::cout << " ";
    //std::cout << "L ";
    print_exptree(ep->getExpression(0), lev + 2); std::cout << " ";
    //std::cout << "R ";
    print_exptree(ep->getExpression(1), lev + 2);
    std::cout << "\n";
    for(i=0; i<slev; i++) std::cout << " ";
    std::cout << ")";
}
