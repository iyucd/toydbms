/* This test and uses index selection and fetch rows*/

select col1, col2, col4, col5 from table1 where col4 = 'Robert' ;

/* This tests '=', '<>', AND, and respecting order column output */

select col5, col4, col3, col2, col1 from table1 where (col2 = 45) AND (col5 <> 'Gonzales') ;

/* This tests '<' and '>' */

select col1, col2, col3, col4, col5 from table1 where (col2 > 45) AND (col5 < 'Gonzales') ;

/* This tests index join */

select col2, col5, col_2, col_3, col_4, col_5 from table2, table1 where (col_2 = col2) AND (col_3 = 'true') ;

/* This tests hash join */

select column1, column2, col1, col2 from table6, table4 where (column3 = col1);

/* This tests nested loop join */

select col1, col2, col3, col_1, col_2, col_3 from table1, table2 where (col3 = col_3) ;

/* This tests cross product */

select col1, col4, col5, col_1, col_4, col_5 from table1, table2 where (col1 > 50) ;

/* This tests support for the IN operator in the WHERE clause */

select col1, col2, col3, col4, col5 from table1 where col4 IN ('Bruce','Isaac','Tom');

/* This tests GROUP BY */

select col1, col2 from table1 group by col2 ;

/* This tests GROUP BY with aggregate function AVG */

select avg(col1) as avg from table1 group by col2 ;

/* This tests GROUP BY with aggregate function COUNT */

select count(col1) as count from table1 group by col2 ;
