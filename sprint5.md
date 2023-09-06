# SP23-141b-Database - Sprint #5
#### Due Sunday May 14, 2023 - 11:30pm (PST)


## Overview -- Let's Select Records!

In this assignment, we are going to implement code to handle "select" commands like, `SELECT * from Users`. When run, this commmand will allow us to retrieve records from our database associated with a given table. as well as ORDER our data, and limit the number of rows we return. This will build on the prior work we've been doing in previous assignments in the `SQLProcessor`, and `Schema` classes. 

> This assignment is slightly bigger, so we'll give you a few extra days to complete the challenges.

Let's get started!


<hr>

## Key Classes in This Assignment 

You'll discover that the following classes are significant in this assignment:

### The `Row` class

As with the case of inserting records, we'll need a working version of our `Row` class to be able select records.

### The `Rows` (or `RowCollection`) class

The `Rows` class will contain 0 or more `Row` objects. This is what actully gets returned by your `Database` object when it performs a `select` command.  

### The `SelectStatement` class

The `SelectStatement` class will perform the common task of helping to parse `SELECT...` commands. You'll implement this class in a manner similar to the other statement classes you have built for previous assignments.   

### The `Storage` class

Since we don't yet have an index, we'll use the `Storage::each()` (visitor) function to iterate blocks in a DB file, and find `Block`s that are associated with a given table/schema. You've probably already used this method for the `DUMP` and `SHOW TABLES` commands. 

### The `DBQuery` class

The _optional_ `DBQuery` class is used to describe (and help exectute) `SELECT`, `UPDATE` and `DELETE` statements.  Note that this class is provided as a starting point. You'll need to develop it further if you want to use them.

### The `Filters` class

The _optional_ `Filters` class can be used to help you deal with queries that include expressions in a "WHERE" clause. This works in conjunction with other classes we've provided started code for -- including `Operand` and `Expression`.  Note that these classes are provided as a starting point. You'll need to develop them further if you want to use them.

### The `ParseHelpers` class

We've provided an updated version of the `ParserHelpers.*` class. This includes tools to aid with dealing with  `SELECT` statements.


### The `TabularView` class

A `TabularView` will be the class that shows a view  of a `RowCollection`  when a user issues `SELECT * ...` command. 

<hr>

## Challenge #1 : Implement the `SelectStatement` Command

The `SELECT` command allows a user to retrieve (one or more) records from a given table. The command accepts one or more fields to be retrieved (or the wildcard `*`), along with a series of **optional** arguments (e.g. `ORDER BY`, `LIMIT`).  For example:
    
```
SELECT * from Accounts order by last_name;
```

When a user issues a `SELECT...` command, you'll create a command class to handle it (e.g. `SelectStatement`), and have it parse the underlying tokens to validate the command. Many of your previous command classes stored state information, to be used when the statement was eventually run in a controller.  Since there are so many options for the `SELECT` command, we recommend storing the options in a call we call `DBQuery`. We'll talk about the many advantages of doing this in lecture. You are free to use this idiom or not.

> We provided you a starter file for the `DBQuery` class.

## Challenge #2 : Write a Controller Function to Load Selected Rows/Fields 

Presuming you can successfully parse  `SELECT` commands, the next step is to evaluate that command, and assemble a collection of `Rows` from your database that match the given query.  This command handler will be attached to one of your controllers like your other commands. We recommend the `Database` object itself,  but the choice is yours.

The key to this challenge is to correctly evaluate the properties you stored when you parsed the `SELECT...` command. There are several facets to consider, as discussed below.

### Loading Rows from Storage

You must be able to access and load all the `Rows` associated with a selected table into memory.  Since we don't yet have indexes, the task of loading rows associated with a table will require that we iterate all storage blocks. Fortunately, our `Storage` class provides the `Storage::each()` (visitor) method for cases of this nature. At this stage, you have to load _all_ the rows associated with a table. Later in this process, you will apply optional filters given to eliminate rows that don't meet given criteria.

For every data `block` you find in storage, verify that the block is associated with the table/schema specified in the `Query`. Decode each of these `Blocks` into a `Row` object, and store each of them in the `Rows` argument (type: `RowCollection`).

## Challenge #3 : Implement the `TabularView` class

In this challenge, you'll construct a `TabularView` to "show" the selected `Rows` to the user. Your `TabularView` should use the `RowCollection` you assembled in the `Database::selectRows()` method as the list of `Row` objects to show to the user. 

### Field Selection

You must be able to select _some_ or _all_ of the fields in a given table, depending on whether the command specified all fields (`SELECT *...`), or individual fields in a command separated field list (`SELECT first_name, last_name from Users...`). How you do this is up to you. You could remove unwanted fields from the `Rows` you've loaded, or deal with this step logically when you create your final `TabularView`.

> NOTE: In this assignment, you don't need to worry about table joins, but you will eventually. It might be helpful to consider the impact of that scenario as you're designing your solution.  For example, if you are JOINing data, you'll likely have to load `RowCollections` for multiple tables, and combine them into a "projected" (or aggregate) table.


## Challenge #4 : Add Database/Controller Logic to Run the "Select" Command

So...you can parse the `SELECT...` statement and create an associated `Command` object?  Great!

Now it's time to implement logic in your controller (e.g. `Database`) to actually perform the action associated with this command.  In our reference implementation, we called this method, `runSelect` -- but you can call it whatever you like.  We stronly encourage you to design your code to use the `DBQuery` class we've started for you (and discussed above and in lecture).  This makes dealing with the set of optional arguments for this command much easier.  

When your controller runs the `runSelect` method, our goal is to generate a list of `Rows` as specified in the query. Since we don't have a real `Index` class yet, you may have to interate your database storage file, in search of blocks that contain data rows that are related to this query.  

As a final step, you'll present the `TabularView` of data `Rows` to the user. Below is a sample of the expected output:

```
SELECT * from Users;
+-------+----------------+----------------+-------+--------+
| id    | first_name     | last_name      | age   | zipcode| 
+-------+----------------+----------------+-------+--------+
| 1     | Terry          | Pratchett      | 81    | 20125  |
+-------+----------------+----------------+-------+--------+
| 2     | Ian            | Tregellis      | 58    | 83311  |
+-------+----------------+----------------+-------+--------+
| 3     | Jodi           | Taylor         | 52    | 48176  |
+-------+----------------+----------------+-------+--------+
3 rows in set (0.000409 secs.)
```

## Challenge #5 : Select...WHERE ... 

This version of the `SELECT` command builds upon the work we've already done. In this case, however, we expect the the records to be filtered according to the given WHERE clause, using a `Filter` object.

```
SELECT * from Users where first_name="Adele";
```

For this task, your code must handle the following tasks (some are repeated from earlier steps):

1. Retrieve `Rows` as a `RowCollection` as you did for the basic `SELECT` case (above)
2. Filter the collection of rows to exlude those that don't pass the conditional logic (using a `Filter`)
3. Present the user data using your `TableView` class in the specified order

> NOTE: You may be asked to handle these optional clauses in any combination or order. For example, the following queries produce identical results:
```
SELECT first_name, last_name from Users LIMIT 3 order by last_name 
SELECT first_name, last_name from Users order by last_name LIMIT 3
```

### Implementing The `Filter` Class

The `Filter` class provides support for conditional logic. We'll use it to perform the conditional checks in the `SELECT..WHERE` clause. The filter class can contain one or more `Expression` objects. An `Expression` contains two `Operands` and one logical `Operator` (e.g. first_name="Megan"). The purpose of this class is to determine whether a row meets the criteria given by the user. Consider the query:

```
SELECT * from Users where age>20 AND zipcode=92100 ORDER BY last_name;
```

The `Filter` class might contain multiple expressions (age>50) (zipcode=92100) and one logical operator (AND). Records from the specified table are loaded (from earlier step), and tested by the given Filter. If a row matches the expression criteria specified in the `Filter`, it will be added to the `RowCollection`.  

We've provided a starter version of the `Filter.hpp` file. You're free to use (or discard) this class. However you will need to implement a `Filter` class so that your code can properly filter records based on values set in the `SELECT` (or other) statements.

> **NOTE**: The `Filter` class can handle basic expressions, but it doesn't (yet) know how to deal logically combining them (AND, OR, NOT).  You and your teammate need to add logic to the `Filter` class to support logically combining expressions. 


### Implementing the Comparison Logic

Also included in this assignment is a file called `Compare.hpp`.  This file provides template functions that implements the "Equals" comparision operator. While we have provided you starting code for the `Equal` condition, you must implement your own variations for the other logical operations (LessThan, GreaterThan, NotEqual, LessThanEqual, GreaterThanEqual,...). 


## Challenge #6 : Select...ORDER BY `fieldname`... 

In this challenge, you'll handle ordering the resulting collection of Rows according to the "ORDER BY" field provided. Your database does not store data in an ordered format -- so we must do this step manually before we present the results to the user. 

```
SELECT last_name, email from Users ORDER BY age;
```

You'll only be asked to order data by a single field, and only in ascending order (smallest first).

## Challenge #7 : Select...LIMIT _N_... 

In this version of the `SELECT` command we are also expected to limit the number of records retrieved. Assuming a table has 100 records, the `LIMIT` clause applies a limit of the total records retrieved, so we might only ask to retrieve N records.

```
SELECT last_name, email from Users LIMIT 5;
```

For this task, your code must handle the following tasks:

1. Retrieve `Rows` as a `RowCollection` as you did for the basic `SELECT` case (above)
2. Stop adding rows once you have reached the given LIMIT value
3. Present the user data using your `TableView` 

<hr>

## Testing This Assignment

For our autograder, we're going to validate your `select` commands are working by retrieving records from a previously created table. Let's assume we already created a Users table and added 2 records. Next we'll issue the `SELECT` command:

```
SELECT * from Users;
```

The output of your select statement should show something like this:

```
> SELECT * from Users;
+--------------------+--------------+
| id  | first_name   | last_name    |
+-----+--------------+--------------+
| 1   | emin         | kirimioglu   |
| 3   | rick         | gessner      |
| 5   | stephen      | kim          |
+-----+--------------+--------------+
3 rows in set (0.00231 sec)
```

We will also test `SELECT` commands with different filters. For instance, we'll issue a command like:

```
> SELECT id, first_name, last_name, zipcode from Users where zipcode>92120 order by zipcode LIMIT 2
+--------------------+--------------+--------------+
| id  | first_name   | last_name    | Zipcode      |
+-----+--------------+--------------+--------------+
| 3   | Anirudh      | Swaninthan   | 92126        |
| 1   | Pu           | Cheng        | 92127        |
+-----+--------------+--------------+--------------+
2 rows in set (0.00123 sec)
```

As always you can use the auto-grader to help get your code to perform as expected.

All of the tests for this assignment will be based on properly gathering the correct records (and number of records) for the given query. The queries won't be any more difficult than the ones shown above. Make sure you compiler output is free from warnings.

#### Grading
```
- Select Test           40 pts
- Filter Test           60 pts
```

## Submitting Your Work by Sunday May 14, 2023 - 11:30pm (PST)
