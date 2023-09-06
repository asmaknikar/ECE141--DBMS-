# SP23-141b-Database - Sprint #3
#### Due Monday, April 24, 11:30pm (PST)

## It's time to start building tables!

In this assignment, we're going to start handling "table-related" commands.  In order to so so, we're going to introduce our 3rd and final CommandProcessor: `SQLProcessor`.  This class will provide support for four new commands:

1. create table `table-name`
2. drop table `table-name`
3. describe `table-name` 
4. show tables

## Integrating given files from assignment #3 

We've added a few new files for this assignment. Just drop them into your existing project.  The following files already exist your project, but these are updates so replace your current version with these:

1. main.cpp
2. TestAutomatic.hpp
3. ScriptRunner
4. TestSequencer

These files all have slight changes in Assignment #3, and should not be overridden.

## New Classes in This Assignment 

### The `Schema` class 

Consider the situation where a user wants to create a database table, by issuing a "create table..." command shown below: 

```
CREATE TABLE tasks (
  id INT AUTO_INCREMENT PRIMARY KEY,
  title VARCHAR(100) NOT NULL,
  price FLOAT DEFAULT 0.0,
  due_date TIMESTAMP, //2020-04-15 10:11:12
  status BOOLEAN DEFAULT FALSE,
)
```

The `Schema` class is an in-memory representation of a table defintion. It's a container class, that contains one-or-more `Attributes`. 
In this example, we defined a table with 5 attributes (task_id, title, price, due_date, status).  We provide you a (nearly) empty version of this file -- and you are free to make any changes you want. 

### The `Attribute` class

An `Attribute` represents a single "field" in a database table defintion (`Schema`). In the table defintion shown above, `task_id` is an example `Attribute`.  The attribute class needs to be able to store the following information:

```
- field_name
- field_type  (bool, float, integer, timestamp, varchar)  //varchar has length
- field_length (only applies to text fields)
- auto_increment (determines if this (integer) field is autoincremented by DB
- primary_key  (bool indicates that field represents primary key)
- nullable (bool indicates the field can be null)
```

We provide you an empty version of this class file, and you are free to make any changes you want. 

### The `Row` class

You are given a new `Row` class. This will hold user data for a given row. The `Row` class will also participate in the storage process. Eventually each `Row` will be stored in a `Block` inside the storage system, and retrieved later to be shown in a `TableView`.  You won't need this until next week.

> Note: At the bottom of `Row.hpp`. we've pre-defined a class called `RowCollection` that you'll use in subsequent assignments. 

> Hint: You may have guessed that `Row`, `Attribute`, and `Schema` are related in many ways. There must be a way of establishing and efficient and meaningful relationship between them.

### The `TabularView` class

A `TabularView` class is used to present user data (rows) as a result of a user issuing a `select...` statement.  You can (but are not required) to use this class to implement the `show tables` command. This class was added here as a placeholder, and to get you started thinking about how you might implement tabular views.

As you're building this class, consider how the `Schema` class might be helpful. Also for previous Views, consider how functionality can be reused from past code or past code can be refactored to reuse this View.

## Implement the Following System Commands 

### Challenge 1: "create table `table-name`" 

When a user issues this command, your "CmdProcessor" will make a corresponding statement object (subclass), that will try to parse the given input. If the input is valid, your code should construct an `Schema` object from that input. Then you'll ask the `Database` to save the given `Schema` in a block within the storage system.  This will result in the `Schema` object being encoded into a `StorageBlock` and saved in a DB file. 

```
> create table test1 (id int NOT NULL auto_increment primary key, first_name varchar(50) NOT NULL, last_name varchar(50));
Query OK, 1 row affected (0.002 sec)
```

One thing to consider -- you will frequently need to retrieve the `Schema` for a given table from storage in order to perform other actions (like inserting records). How will you quickly find and retrieve the `Schema` for a given table from storage when you need it later?  You could, of course, perform a linear scan of every `Block` in the db storage file and try to locate the `Schema` for a named table. But that would be terribly slow in a large database.  Work with your partner (or your TA) to consider your options on ways to solve this. In class, we discussed the possibility of using an `Index` class that maps key/value pairs for each schema (key=schema_name, value=block_number);

> Note: When you save an `Schema` in the storage file, the associated "block-type" will be a "SchemaBlock". If you were to use the `dump database {name}` command from last week, an "SchemaBlock", make sure your output can distinguish an "SchemaBlock" from a "Meta" or "Data" block by properly setting the `Block.header.type` field.

#### Parsing `Create Table`...

Parsing the `create table foo ()` command will be more challenging that other commands you've processsed so far. Fortunately, it follows a simple pattern shown as tokens. So...we see "create", "table", "{tablename}", "(" -- followed by list of attributes, then closed with ");". We definitely want to build little helper classes to deal with these things. Possibly the `Statement` classes mentioned in lecture?

```
ATTRIBUTE is...
  [name] [type] [optional args] 
  
CREATE TABLE is...
[create] [table] [(] 
  [attribute] [optional comma]
    ...more attributes separated by commas...
[)] [;]
```

#### Error Handling 

If by chance a given command is malformed or incomplete, you must return a well-formed error message. Here's an example:

```
create table 0123foo (id int auto_increment primary key, name varchar(50));    //NOTE BAD TABLE NAME!
Error 101: Identifier expected at line 1
```

Your error message must include the error code, message string, and line number (of the input script).  It's ok if you just use 1 for the line number for now.  

NOTE: Part of the grading for this assignment will be to test your error handling during parsing.


### Challenge 2: "show tables" 

As we discussed in class -- when a user issues this command in a SQL database, the list of known tables (from the database in use) are shown to the user in the terminal.  You will implement that now.   Let's assume that your user created a few tables. Your output should be similar to what you see in MySQL:

```
> show tables;
+----------------------+
| Tables_in_mydb       |
+----------------------+
| groups               |
| users                |
+----------------------+
2 rows in set (0.000025 sec.)
```

> Note: This output looks similar to something else we've outputted. Possibly that could be reused.

1. Create a `ShowTables` class that can recognize and parse this type of command
2. Have one of your command processors handle this command, create the `ShowTables` statement object, and handle routing as usual 
3. If all goes well, ask a controller object (maybe the `Database`?) to exectute the command and get the list of known `Schemas` 
4. Consider building a `TableView` class, and let that present a view that contains the list of tables in the DB 

### Challenge 3: "drop table `table-name`"

When your user issues this command, then your system should attempt to locate and delete the associated `Schema` (table) from the storage system. If the table doesn't exist, report an "unknown table" error to your user.  

```
> drop table groups;
Query OK, 0 rows affected (0.02 sec)
```

> **NOTE:** The number of rows affected depends on how many data rows are stored in the database for that table. 

As we discussed in lecture, "deleting" a `Block` means to set (and save) the `Block.header.type`='F' (for free). 

> NOTE: If a user deletes a table that has associated records (or indexes), these elements must also be deleted from Storage. This isn't a concern today, because we can't add records yet. But give this some consideration for a future assignment.

1. Create a `DropTableStatement` class (subclass of `Statement`) that can parse this syntax
2. Let a given CommandProcessor process and route this command
3. Assuming this given `table-name` actually exists, ask the `Database` object to delete that `Schema` and associated data.
4. Report the results back to your user (see example above)

### Challenge 4: "describe `table-name`"

When your user issues this command, then your system should attempt to locate and describe the associated `Schema`. If the table doesn't exist, report an "unknown table" error to your user.

As we discussed in lecture, "describing" a `Schema` means to print out a textual description of the attributes of the given table. For example, if we were to issue the command "describe tasks" (using the table description above) we expect to see. Note, the columns are expectd, but presentational decisions (like the widths of the columns) are up to you.

```
> DESCRIBE tasks;
+-----------+--------------+------+-----+---------+-----------------------------+
| Field     | Type         | Null | Key | Default | Extra                       |
+-----------+--------------+------+-----+---------+-----------------------------+
| id        | integer      | NO   | YES | NULL    | auto_increment primary key  |
| title     | varchar(100) | NO   |     | NULL    |                             |
| price     | float        | YES  |     | 0.0     |                             |
| due_date  | date         | YES  |     | NULL    |                             |
| status    | boolean      | YES  |     | FALSE   |                             |
+-----------+--------------+------+-----+---------+-----------------------------+
5 rows in set (0.000043 sec.)
```

1. Create a Command/Statement class that that can parse this statement
2. Let one of your command-processors route this command
3. Assuming this given `table-name` actually exists, ask the `Database` object to handle this command 
4. Consider creating a `SchemaView` to show the table description (shown above)


## Testing

As before, we recommend you perform testing on your solution by creating a test script. You can also use the auto-grader provided on github classroom for this assignment.

> **NOTE:**: Update the output value for your version command, so to that this application is version 1.3. 

## Grading

```
Parse-test: 20pts  //full of errors you need to report...
Table-test: 80pts
```

## Submitting Your Work by Monday, April 24, 11:30pm (PST)
You and your partner should make sure you turn your solution in on time!  

Good luck everyone!
