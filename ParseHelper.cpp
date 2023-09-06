//
//  ParseHelpers.cpp
//  RGAssignment4
//
//  Created by rick gessner on 4/18/21.
//

#include "ParseHelper.hpp"
#include "Helpers.hpp"
#include "Schema.hpp"
#include "Database.hpp"

namespace ECE141 {
  
// USE: parse table name (identifier) with (optional) alias...
  StatusResult ParseHelper::parseTableName(std::string& aTableName){
  
    StatusResult theResult{Errors::identifierExpected};
    Token &theToken=tokenReader.current(); //get token (should be identifier)
    if(TokenType::identifier==theToken.type) {
      //aTableName.table=theToken.data;
      aTableName = theToken.data;
      theResult.error=Errors::noError;
      tokenReader.next(); //skip ahead...
      //if(tokenReader.skipIf(Keywords::as_kw)) { //try to skip 'as' for alias...
      //  Token &theToken=tokenReader.current();
      //  aTableName.alias=theToken.data; //copy alias...
      //  tokenReader.next(); //skip past alias...
      //}
    }
    return theResult;
  }
  static ClauseHandlers clauseHandlers{
            {Keywords::where_kw,[](CommandTokenReader& aCommandTokenReader, Schema aSchema, Clauses& aClauses) {
                aCommandTokenReader.next();
                Filters theFilters;
                StatusResult theResult;
                theResult = theFilters.parse(aCommandTokenReader,aSchema);
                aClauses.setFilters(theFilters);
                return theResult;
            }},
            {Keywords::order_kw,[](CommandTokenReader& aCommandTokenReader, Schema aSchema, Clauses& aClauses) {
                aCommandTokenReader.next(2);
                //parsing expression
                ParseHelper aHelper(aCommandTokenReader);
                Order theOrder;
                StatusResult theResult = aHelper.parseOrder(theOrder);
                aClauses.setOrderingVar(theOrder);
                return theResult;
            }},
            {Keywords::limit_kw,[](CommandTokenReader& aCommandTokenReader, Schema aSchema, Clauses& aClauses) {
                if (aCommandTokenReader.remaining() < 2) return StatusResult(Errors::invalidArguments);
                aCommandTokenReader.next();
                ind theLimit = std::stoi(aCommandTokenReader.current().data);
                aCommandTokenReader.next();
                aClauses.setLimit(theLimit);
                return StatusResult();
            }},
  };
  StatusResult ParseHelper::parseClauses(Schema_ptr aSchema, Clauses& aClauses) {
      StatusResult theResult;
      while (tokenReader.more()) {
          theResult = clauseHandlers[tokenReader.current().keyword](tokenReader, *aSchema, aClauses);
      }
      return theResult;
  }

  StatusResult ParseHelper::parseQuery(DBQuery& aQuery) {
      StatusResult theResult;
      return theResult;
  }

  StatusResult ParseHelper::parseTableField(TableField &aField) {
    StatusResult theResult{Errors::identifierExpected};
    Token &theToken=tokenReader.current(); //identifier name?
    if(TokenType::identifier==theToken.type) {
      aField.fieldName=theToken.data;
      tokenReader.next();
      if(tokenReader.more() && tokenReader.skipIf(Operators::dot_op)) {
        theToken=tokenReader.current();
        if(TokenType::identifier==theToken.type) {
          tokenReader.next(); //yank it...
          aField.table=aField.fieldName;
          aField.fieldName=theToken.data;
        }
      }
      theResult.error=Errors::noError;
    }
    return theResult;
  }

  // USE: gets properties following the type in an attribute decl...
  StatusResult ParseHelper::parseAttributeOptions(Attribute &anAttribute) {
    bool          options=true;
    StatusResult  theResult;
    //char          thePunct[]={"),"}; //removed semi?
    
    while(theResult && options && tokenReader.more()) {
      Token &theToken=tokenReader.current();
      switch(theToken.type) {
        case TokenType::keyword:
          switch(theToken.keyword) {
            case Keywords::auto_increment_kw:
              anAttribute.setAutoIncrement(true);
              break;
            case Keywords::primary_kw:
              anAttribute.setPrimaryKey(true);
              break;
            case Keywords::not_kw:
              tokenReader.next();
              theToken=tokenReader.current();
              if(Keywords::null_kw==theToken.keyword) {
                anAttribute.setNullable(false);
              }
              else theResult.error=Errors::syntaxError;
              break;
              
            default: break;
          }
          break;
          
        //case TokenType::punctuation: //fall thru...
        //  options=!in_array<char>(thePunct,theToken.data[0]);
        //  if(semicolon==theToken.data[0])
        //    theResult.error=Errors::syntaxError;
        //  break;
                  
        default:
          options=false;
          theResult.error=Errors::syntaxError;
      } //switch
      if(theResult) tokenReader.next(); //skip ahead...
    } //while
    return theResult;
  }
    
  //USE : parse an individual attribute (name type [options])
  StatusResult ParseHelper::parseAttribute(Attribute &anAttribute) {
    StatusResult theResult;
    
    if(tokenReader.more()) {
      Token &theToken=tokenReader.current();
      if(Helpers::isDatatype(theToken.keyword)) {
        DataTypes theType = Helpers::keywordToDatatype(theToken.keyword);
        anAttribute.setDataType(theType);
        tokenReader.next();
        
        if(DataTypes::varchar_type==theType) {
          if((tokenReader.skipIf(left_paren))) {
            theToken=tokenReader.current();
            tokenReader.next();
            if((tokenReader.skipIf(right_paren))) {
              anAttribute.setSize(atoi(theToken.data.c_str()));
              // return theResult;
            }
          }
        }
        
        if(theResult) {
          theResult=parseAttributeOptions(anAttribute);
          if(theResult) {
            if(!anAttribute.isValid()) {
              theResult.error=Errors::invalidAttribute;
            }
          }
        }
        
      } //if
      else theResult.error=Errors::unknownType;
    } //if
    return theResult;
  }
  StatusResult ParseHelper::parseFromTable(std::string& aTableName) {
      StatusResult theResult;
      if (!tokenReader.skipIf(Keywords::from_kw)) theResult.error = Errors::invalidCommand;
      aTableName = tokenReader.current().data;
      if (tokenReader.more()) tokenReader.next();
      return theResult;
  }
  //USE: parse a comma-sep list of (unvalidated) identifiers;
  //     AUTO stop if keyword (or term)
  StatusResult ParseHelper::parseIdentifierList(StringList &aList) {
    StatusResult theResult;
    tokenReader.skipIf(left_paren);
    while(theResult && tokenReader.more()) {
      Token &theToken=tokenReader.current();
      if(TokenType::identifier==tokenReader.current().type) {
        aList.push_back(theToken.data);
        tokenReader.next(); //skip identifier...
        tokenReader.skipIf(comma);
      }
      else if(theToken.type==TokenType::keyword) {
        break; //Auto stop if we see a keyword...
      }
      else if(tokenReader.skipIf(right_paren)){
        break;
      }
      else if(semicolon==theToken.data[0]) {
        break;
      }
      else theResult.error=Errors::syntaxError;
    }
    return theResult;
  }

  //** USE: get a list of values (identifiers, strings, numbers...)
  StatusResult ParseHelper::parseValueList(StringList &aList) {
    StatusResult theResult;
    
    while(theResult && tokenReader.more()) {
      Token &theToken=tokenReader.current();
      if(TokenType::identifier==theToken.type || TokenType::number==theToken.type) {
        aList.push_back(theToken.data);
        tokenReader.next(); //skip identifier...
        tokenReader.skipIf(comma);
      }
      else if(tokenReader.skipIf(right_paren)) {
        break;
      }
      else theResult.error=Errors::syntaxError;
    }
    return theResult;
  }

  bool isDotOperator(const Token &aToken) {
    if(TokenType::operators==aToken.type) {
      return aToken.op==Operators::dot_op;
    }
    return false;
  }

  const Attribute getAttribute(Schema &aSchema, const std::string &aName,
                              const std::string &aFieldName) {
    Schema *theSchema=&aSchema;
    if(aSchema.getName()!=aName) {
      //theSchema=aSchema.getDatabase().getSchema(aName);
    }
    return theSchema ? theSchema->getAttribute(aFieldName) : Attribute();
  }

  //where operand is field, number, string...
   StatusResult ParseHelper::parseOperand(Schema &aSchema,
                                          Operand &anOp) {
     StatusResult theResult;
     Token &theToken =tokenReader.current();
     if(TokenType::identifier==theToken.type) {
       std::string theEntityName(aSchema.getName());
       if(tokenReader.remaining()>1) {
         Token &theNext = tokenReader.peek();
         if (isDotOperator(theNext)) {
           theEntityName = theToken.data;
           tokenReader.next(2);
           theToken = tokenReader.current();
         }
       }
       auto theAttr = getAttribute(aSchema, theEntityName, theToken.data);
       if(theAttr.isValid()) {
         anOp.setAttribute(
            theToken,aSchema.getHashedName(), theAttr.getType());
       }
       else anOp.setVarChar(theToken.data);
     }
     else if(TokenType::number==theToken.type) {
       anOp.setNumber(theToken);
       anOp.name = theToken.data;
     }
     else theResult.error=Errors::syntaxError;
     if(theResult) tokenReader.next();
     return theResult;
   }

   StatusResult ParseHelper::parseOperator(Operators &anOp) {
     static std::vector<Operators> gOps={
       Operators::equal_op, Operators::notequal_op,
       Operators::lt_op, Operators::lte_op,
       Operators::gt_op, Operators::gte_op
     };
     
     StatusResult theResult{Errors::operatorExpected};
     Token &theToken=tokenReader.current();
     if(Helpers::in_array(gOps, theToken.op)) {
       anOp=theToken.op;
       tokenReader.next();
       theResult.error=Errors::noError;
     }
     return theResult;
   }
   StatusResult ParseHelper::parseLogical(Logical& aLogic) {
       std::map<Keywords, Logical> logicMap{
           {Keywords::and_kw,Logical::and_op},
           {Keywords::or_kw,Logical::or_op},
           {Keywords::not_kw,Logical::not_op},
       };
       StatusResult theResult;
       aLogic = logicMap[tokenReader.current().keyword];
       tokenReader.next();
       return theResult;
   }

   StatusResult ParseHelper::parseExpression(Schema &aSchema,
                                             Expression &anExpr) {
     StatusResult theResult;
     
     if((theResult=parseOperand(aSchema,anExpr.lhs))) {
       if((theResult=parseOperator(anExpr.op))) {
         theResult=parseOperand(aSchema,anExpr.rhs);
       }
       else theResult.error=Errors::operatorExpected;
     }
     return theResult;
   }

   StatusResult ParseHelper::parseOrder(Order& anOrder) {
       StatusResult theResult;
       TableField aField;
       theResult = parseTableField(aField);
       anOrder.orderingColumn = aField;
       if (tokenReader.more()) {
           if (tokenReader.skipIf(Keywords::asc_kw)) anOrder.descending = false;
           else if (tokenReader.skipIf(Keywords::desc_kw)) anOrder.descending = true;
       }
       return StatusResult();
   }
   StatusResult ParseHelper::parseJoin(Join& aJoin) {
       Token& theToken = tokenReader.current();
       StatusResult theResult(Errors::joinTypeExpected); //add joinTypeExpected to your errors file if missing...
       Keywords theJoinType{ Keywords::join_kw }; //could just be a plain join
       if (Helpers::in_array(gJoinTypes, theToken.keyword)) {
           theJoinType = theToken.keyword;
           tokenReader.next(1); //yank the 'join-type' token (e.g. left, right)
           if (tokenReader.skipIf(Keywords::join_kw)) {
               std::string theTable;
               if ((theResult = parseTableName(theTable))) {
                   aJoin = Join(theTable, theJoinType, std::string(""), std::string(""));
                   theResult = Errors::noError; //on...
                   if (tokenReader.skipIf(Keywords::on_kw)) { //LHS field = RHS field
                       TableField LHS(" ");
                       if ((theResult = parseTableField(aJoin.onLeft))) {
                           if (tokenReader.skipIf(Operators::equal_op)) {
                               theResult = parseTableField(aJoin.onRight);

                           }
                       }
                   }
               }
           }
       }
       return theResult;
   }

  //read a comma-sep list of expressions...
  StatusResult ParseHelper::parseAssignments(Expressions &aList,
                                             Schema &aSchema){
    StatusResult theResult;
    while(theResult && tokenReader.more()) {
      Expression theExpr;
      if((theResult=parseExpression(aSchema, theExpr))) {
        aList.push_back(std::make_unique<Expression>(theExpr));
        if(!tokenReader.skipIf(',')) {
          break;
        }
      }
      else theResult.error=Errors::syntaxError;
    }
    return theResult;
  }


}
