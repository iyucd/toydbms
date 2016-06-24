#include <iostream>
#include <cstdlib>
#include <cstring>
#include "sql_driver.h"
#include "globals.h" 
#include "Table.h"
using namespace std;
extern string load_row_lookup_table();
extern string load_btree_indexes();
extern string load_primary_keys();
extern string load_keys();
extern string load_stats();
extern vector<Table> tables;
extern bool file_data_loaded;
extern bool print_error_message;
extern stack<string> symbol_stack;

int
main( const int argc, const char **argv )
{
    /** check for the right # of arguments **/
    if( argc >= 2 )  {
        load_row_lookup_table();
        load_btree_indexes();
        load_primary_keys();
        load_keys();
        load_stats();
        // Set flags so we know a tables information has been loaded

        

        for (int i = 0; i < tables.size(); i++) {
            // Set flags so we know a tables information has been loaded
            tables[i].SetPrimaryKeyFlagsLoaded(true);
            tables[i].SetKeyFlagsLoaded(true);
            tables[i].SetBtreesLoaded(true);
            // If a tables btrees did not get set up due to no .idx file,
            // create empty trees for future use.
            if (tables[i].indexes.size() == 0) {
                stx::btree<string, int> new_tree;
                for (int j = 0; j < tables[i].GetColumnNamesVecSize(); j++)
                    tables[i].indexes.push_back(new_tree);
            }
        } 

        file_data_loaded = false;
        print_error_message = true; 
        if( std::strncmp( argv[ 1 ], "-h", 2 ) == 0 ) {   /** simple help menu **/
            std::cout << "use -o for pipe to std::cin\n";
            std::cout << "just give a filename to count from a file\n";
            std::cout << "use -h to get this menu\n";
            return( EXIT_SUCCESS );
        }
        UCD::SQLDriver driver;
        /** example for piping input from terminal, i.e., using cat **/
        if( std::strncmp( argv[ 1 ], "-f", 2 ) == 0 ) {
            /** example reading input from a file **/
            /** assume file, prod code, use stat to check **/
            driver.parse( argv[1] );
        } else {
            while(true) {
                std::cout << "$> ";
                driver.parse( std::cin );
                if(driver.quit()) break;
            }
            std::cout << "Quitting...\n";
        }
        //driver.print( std::cout ) << "\n";
    } else {
        /** exit with failure condition **/
        return ( EXIT_FAILURE );
    }
    return( EXIT_SUCCESS );
}
