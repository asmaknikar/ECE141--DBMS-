# SP23-141b-Database - Sprint #8
#### Due Monday May 29, 2023 - 11:30pm (PST)

This assignment will come in two parts. In the first part, you will be implementing **Table Joins** and in the second part, you will be implementing **Caching**. You will have a longer amount of time to complete both parts, and as always, start early!

## Part One - Table Joins

A relational database consists of multiple related tables, that can be linked together using matching column values. Because of this, data in each individual table is often incomplete from the business perspective.  By keeping distinct data in separate tables, we gain clarity and better data management. We can recombine data from multiple tables to ask more complex questions. In this part, you'll implement the logic needed to perform a LEFT JOIN, or a RIGHT JOIN.

As we discussed in lecture, there are many types of table joins: `inner, full, left, right, self, cross`. In this part, we'll focus on `LEFT` joins.  Please refer to the recent lecture on joins, and our "database explainer video" for more details.

<hr>

## Key Types in This Part 

You'll discover that the following classes are significant in this part:

### The `Join` class

The `Join` class is a small "helper" for the `SelectStatement` class. Use this to hold information for the `SELECT` query that includes a JOIN clause, like situations below:

```
SELECT users.first_name, users.last_name, order_number 
FROM users
LEFT JOIN orders ON users.id=orders.user_id
```

The `JOIN` class holds the data you gather if your SELECT statement includes a JOIN clause.  Mainly, this includes the table(s) you are joining together, the JOIN type, and the fields from each table in the join (the 'ON' expression).

### The `gJoinTypes` array

To help with parsing Joins, we use an array called `gJoinTypes` that defines a set of `Keywords` that specify the join types. If you like, you can use this array during parsing of the `JOIN` clause.  It's up to you to make sure each of these keywords is in your keywords list. 

```
  static ECE141::Keywords gJoinTypes[]={
    ECE141::Keywords::cross_kw, ECE141::Keywords::full_kw, ECE141::Keywords::inner_kw,
    ECE141::Keywords::left_kw,  ECE141::Keywords::right_kw
  };
```

### The `in_array` function

This `in_array` template function let us do a quick query to determine if a keyword is included in an array of keywords (such as `gJoinTypes` above). Make sure this function is included in your `Helpers.hpp` file.  We use this function in the `parseJOIN` method. It's also a really cool idiom -- because the function can automatically determine the size of the array you pass to the function!

```
  template<typename T, size_t aSize>
  bool in_array(T (&anArray)[aSize], const T &aValue) {
    return std::find(std::begin(anArray), std::end(anArray), aValue);
  };
  
  //example usage in parseJoin method...
  if(in_array<Keywords>(gJoinTypes, theToken.keyword)) {
    //do something...
  }
```

### The `parseJoin` method

Below, we're providing an outline of a new method, `paresJoin` that you can include in your code for use in parsing `SELECT` statements.  This method will be called by your `SelectStatement::parse()` method, if/when it encounters a JOIN type. This indicates that your `SELECT` statement has a `JOIN` clause, and that this method should be called to handle it.  

> NOTE: You may have to adapt this method to work in your code. However, it offers a reasonable starting place.

```
  //jointype JOIN tablename ON table1.field=table2.field
  StatusResult SelectStatement::parseJoin(Tokenizer &aTokenizer) {
    Token &theToken = aTokenizer.current();
    StatusResult theResult{joinTypeExpected}; //add joinTypeExpected to your errors file if missing...

    Keywords theJoinType{Keywords::join_kw}; //could just be a plain join
    if(in_array<Keywords>(gJoinTypes, theToken.keyword)) {
      theJoinType=theToken.keyword;
      aTokenizer.next(1); //yank the 'join-type' token (e.g. left, right)
      if(aTokenizer.skipIf(Keywords::join_kw)) {
        std::string theTable;
        if((theResult=parseTableName(aTokenizer, theTable))) {
          Join theJoin(theTable, theJoinType, std::string(""),std::string(""));
          theResult.code=keywordExpected; //on...
          if(aTokenizer.skipIf(Keywords::on_kw)) { //LHS field = RHS field
            TableField LHS("");
            if((theResult=parseTableField(aTokenizer, theJoin.lhs))) {
              if(aTokenizer.skipIf(Operators::equal_op)) {
                if((theResult=parseTableField(aTokenizer, theJoin.rhs))) {
                  joins.push_back(theJoin);
                }
              }
            }
          }
        }
      }
    }
    return theResult;
  }
```

> NOTE: This method includes calls to the method, `parseTableField` -- a method you need to provide yourself. This method tries to parse the name of a table for your statement. You may already have a similar method, and can substitute that instead.

<hr>

### Integrating the `JOIN` class

Add the `JOIN` class to your `SelectStatement.hpp` file (or whatever you called it)

```
struct Join  {
    Join(const std::string &aTable, Keywords aType, const std::string &aLHS, const std::string &aRHS)
      : table(aTable), joinType(aType), onLeft(aLHS), onRight(aRHS) {}
        
    Keywords    joinType;
    std::string table;
    TableField  onLeft;
    TableField  onRight;
  };  
```

Next, in your `SelectStatement` class, add a new data member:
```
class SelectStatement {
  //your existing stuff here...
  
protected:
  //all your existing members...
  std::vector<Join> joins;
};
```

<hr>

## Implementing  `LEFT JOIN` 

### For the following examples, we created two tables: books and authors:

```
> select * from Authors;
+----+------------+-----------+
| id | first_name | last_name |
+----+------------+-----------+
|  1 | Stephen    | King      |
|  2 | JK         | Rowling   |
|  3 | Truong     | Nguyen    |
+----+------------+-----------+
3 rows in set (0.00 sec)

> select * from Books;
+----+-------------------------------------------+-----------+
| id | title                                     | author_id |
+----+-------------------------------------------+-----------+
|  1 | Harry Potter and the Sorcerer's Stone     |         2 |
|  2 | Harry Potter and the Philosopher's Stone  |         2 |
|  3 | Harry Potter and the Prisoner of Azkaban  |         2 |
|  4 | Harry Potter and the Chamber of Secrets   |         2 |
|  5 | Harry Potter and the Goblet of Fire       |         2 |
|  6 | Harry Potter and the Order of the Phoenix |         2 |
|  7 | Harry Potter and the Half-Blood Prince    |         2 |
|  8 | Carrie                                    |         1 |
|  9 | The Dark Tower                            |         1 |
| 10 | The Green Mile                            |         1 |
| 11 | Wavelets and Filter Banks                 |         0 |
+----+-------------------------------------------+-----------+
11 rows in set (0.00 sec)
```

#### Implementation Details

As we discussed in lecture, a `LEFT JOIN` selects data starting from the left table. For each row in the left table, the left join compares with every row in the right table. If the values in the two rows cause the join condition evaluates to true (we find matches), the left join creates a new row whose columns contain all columns of the rows in both tables and includes this row in the result set.

If the values in the two rows are not matched, the left join clause still creates a new row whose columns contain specified fields of the row in the left table and NULL for fields specified for the right table.

In other words, the `LEFT JOIN` selects all data from the left table whether there are matching rows exist in the right table or not. In case there are no matching rows from the right table found, NULLs are used for columns of the row from the right table in the final result set.

> NOTE: Remember that it's possible to have authors who haven't had a book published (yet)


#### Based on this data, here are the results from a LEFT join:

```
> select last_name, title from Authors left join Books on Authors.id=Books.author_id;
+-----------+-------------------------------------------+
| last_name | title                                     |
+-----------+-------------------------------------------+
| Rowling   | Harry Potter and the Sorcerer's Stone     |
| Rowling   | Harry Potter and the Philosopher's Stone  |
| Rowling   | Harry Potter and the Prisoner of Azkaban  |
| Rowling   | Harry Potter and the Chamber of Secrets   |
| Rowling   | Harry Potter and the Goblet of Fire       |
| Rowling   | Harry Potter and the Order of the Phoenix |
| Rowling   | Harry Potter and the Half-Blood Prince    |
| King      | Carrie                                    |
| King      | The Dark Tower                            |
| King      | The Green Mile                            |
| Nguyen    | NULL                                      |
+-----------+-------------------------------------------+
11 rows in set (0.00 sec)
```
<hr>

## Implementing  `RIGHT JOIN` 

#### Based on the data provided (above), here are the results from a RIGHT JOIN:

```
> select last_name, title from Authors right join Books on Authors.id=Books.author_id;
+-----------+-------------------------------------------+
| last_name | title                                     |
+-----------+-------------------------------------------+
| King      | Carrie                                    |
| King      | The Dark Tower                            |
| King      | The Green Mile                            |
| Rowling   | Harry Potter and the Sorcerer's Stone     |
| Rowling   | Harry Potter and the Philosopher's Stone  |
| Rowling   | Harry Potter and the Prisoner of Azkaban  |
| Rowling   | Harry Potter and the Chamber of Secrets   |
| Rowling   | Harry Potter and the Goblet of Fire       |
| Rowling   | Harry Potter and the Order of the Phoenix |
| Rowling   | Harry Potter and the Half-Blood Prince    |
| NULL      | Wavelets and Filter Banks                 |
+-----------+-------------------------------------------+
11 rows in set (0.00 sec)
```

As we discussed in class,  `RIGHT JOIN` clause is similar to the `LEFT JOIN` clause except that the treatment of tables is reversed. The `RIGHT JOIN` starts selecting data from the right table instead of the left table. We select all rows from the right table and match rows in the left table. If a row from the right table does not have matching rows from the left table, fields specified from of the left table will have NULL in the final result set.

## Testing This Part

### LEFT JOIN vs RIGHT JOIN 

In our automated test, we provide a testing method called `doJoinTest` which will test your `LEFT JOIN`:

```
select first_name, last_name, title from Users left join Books on Users.id=Books.user_id order by last_name;
```

You'll need to do manual testing to validate your code for the `RIGHT JOIN`:

```
select first_name, last_name, title from Books left join Users on Books.user_id=Users.id order by last_name;
```

## Part Two - Adding a Cache for Performance

In this part, we are going to add a cache to improve overall system performance. 

<hr>

## Key Classes in This Part : `LRUCache`

The main puprose of the `LRUCache` class is to hold a set of data in memory (avoiding disk operations) so that you can improve overall system performance. You'll want to adapt your code slightly to support the cache (for reading and writing). 

We've provided a minimal version of the `LRUCache` class. It's up to you to adapt and implement this code to meet your needs. You can change it any way you like, or replace all of it with your own.

```
  template<typename KeyT, typename ValueT>
  class LRUCache {
  public:
        
    //OCF 

    void    put(const KeyT &key, const ValueT& value);
    ValueT& get(const KeyT& key);    
    bool    contains(const KeyT& key) const;
    size_t  size() const; //current size
    
  protected:
    size_t maxsize; //prevent cache from growing past this size...

    //data members here...
  };
```

You should design your `LRUCache` so that can cache `Block` or `Row` objects. For `Block` objects, the cache "key" will likely be the block number, and the value an actual `Block`. For a `Row` cache, you will likely use a primary key (int) for the "key", and a `Row` or `Row*` for the value.

> **NOTE:** The `Config` class has settings that determine the size of the cache you should use. If that value is 0, then the cache should be disabled. You can call `getCacheSize(CacheType::block)` to get this value, or call `useCache(CacheType::block)` to just determine if the cache should be enabled.

```
static size_t getCacheSize(CacheType aType);
static void   setCacheSize(CacheType aType, size_t aSize);
static bool   useCache(CacheType aType);
```

> **NOTE:** The size of your `LRUCache` memory cache should be obtained at run-time by calling `Config::getCacheSize(CacheType)`. This approach allows our testing system to test performance of your system using different size caches.  Caching should only be enabled for a given `CacheType` (block, row, view) if `Config::getCacheSize(CacheType)` returns a non-zero value. 

<hr>

## Adding Support For the `LRUCache` Class

We are providing you a basic `LRUCache` implementation, and you are free to use this, or implement you own.

Integrate the `LRUCache` into the flow of your logic. However, recall that we only _use_ the cache if the `Config::getCacheSize(CacheType)` returns a non-zero value.  `Config::CacheType` argument may be 'block', 'rows', or 'views'. 

### Challenge 1 

In this challenge you'll only implement a block cache. We discussed several ways you can implement this in your system. The only hard requirement is that your code must be able to run with the cache enabled or disabled. The testing system will vary this `Config` setting. 

When enabled, you want to load blocks into your cache, so that they may be retrieve more quickly on a subsequent request. Your caching policy algorithm should be designed to keep "most-recently-used" blocks, and discard the "least recently used". If block requests exceed the capacity of your cache, you may have to discard recently used blocks. 

### Challenge 2 -- Optional

In this challenge, you'll implement a cache for `Row` objects. The template we provided you as a starter for your `LRUCache` can be used for either blocks or rows -- given the correct set of template arguments.  So this challenge involves integrating the cache with your logic for reading/writing `Row` objects. 

### Challenge 3 -- Optional

In this challenge -- you'll add a "view" cache to your system. Since views are text based, you can cache your views on disk, and reuse them (by loading the view output text from disk) if the same query is sent to your database.  You can also use the `LRUCache` to cache views in memory if you prefer.  

## Testing This Assignment

As always you can use the auto-grader to help get your code to perform as expected.

```
- Joins Test 70pts
- Block Cache Test 30pts
---- Extra Credit ----
- Row Cache Test 15pts
- View Cache Test 15pts
```

## Turning in your work by Monday May 29, 2023 - 11:30pm (PST)

Make sure your code compiles, and meets the requirements given above.

