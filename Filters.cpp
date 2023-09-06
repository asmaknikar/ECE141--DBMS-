//
//  Filters.cpp
//  Datatabase5
//
//  Created by rick gessner on 3/5/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <limits>
#include "Filters.hpp"
#include "keywords.hpp"
#include "Helpers.hpp"
#include "Schema.hpp"
#include "Attribute.hpp"
#include "ParseHelper.hpp"
#include "Compare.hpp"

namespace ECE141 {
  
  using Comparitor = bool (*)(const Value& aLHS, const Value& aRHS);
  using LogicOp = bool (*)(bool aLHS, bool aRHS);

  bool Expression::equals(const Value& aLHS, const Value& aRHS) {
    bool theResult=false;
    
    std::visit([&](auto const &aLeft) {
      std::visit([&](auto const &aRight) {
        theResult=isEqual(aLeft,aRight);
      },aRHS);
    },aLHS);
    return theResult;
  }
  bool Expression::notEquals(const Value& aLHS, const Value& aRHS) {
      return !equals(aLHS, aRHS);
  }

  bool Expression::lessThan(const Value& aLHS, const Value& aRHS) {
      bool theResult = false;

      std::visit([&](auto const& aLeft) {
          std::visit([&](auto const& aRight) {
              theResult = isLessThan(aLeft, aRight);
              }, aRHS);
          }, aLHS);
      return theResult;
  }
  bool Expression::lessThanEqualTo(const Value& aLHS, const Value& aRHS) {
      return lessThan(aLHS, aRHS) || equals(aLHS, aRHS);
  }
  
  bool Expression::greaterThan(const Value& aLHS, const Value& aRHS) {
      bool theResult = false;

      std::visit([&](auto const& aLeft) {
          std::visit([&](auto const& aRight) {
              theResult = isGreaterThan(aLeft, aRight);
              }, aRHS);
          }, aLHS);
      return theResult;
  }
  bool Expression::greaterThanEqualTo(const Value& aLHS, const Value& aRHS) {
      return greaterThan(aLHS, aRHS) || equals(aLHS, aRHS);
  }

  static std::map<Operators, Comparitor> comparitors{
    {Operators::equal_op, Expression::equals},
    {Operators::notequal_op, Expression::notEquals},
    {Operators::lt_op, Expression::lessThan},
    {Operators::lte_op, Expression::lessThanEqualTo },
    {Operators::gt_op, Expression::greaterThan},
    {Operators::gte_op, Expression::greaterThanEqualTo },
  
     /*   between_op,
    or_op, nor_op, and_op, not_op, dot_op,
    add_op, subtract_op, multiply_op, divide_op, power_op, mod_op,
    unknown_op*/
  };
  static std::map<Logical, LogicOp> logicalOps{
      {Logical::no_op, [](bool arg1,bool arg2) {return arg1 && arg2;}}, // default and together expressions
      {Logical::and_op, [](bool arg1, bool arg2) {return arg1 && arg2; }},
      {Logical::not_op, [](bool arg1, bool arg2) {return !arg2; }},
      {Logical::or_op, [](bool arg1, bool arg2) {return arg1 || arg2; }},
      {Logical::unknown_op, [](bool arg1, bool arg2) {return false; }},
  };
  bool Expression::operator()(const RowKeyValues& aList) {
    //STUDENT: Add code here to evaluate the expression...
    Comparitor compare = comparitors[op];
    Value theLHS = lhs.value;
    Value theRHS = rhs.value;
    if (lhs.ttype == TokenType::identifier) theLHS = aList.at(lhs.name);
    if (rhs.ttype == TokenType::identifier) theRHS = aList.at(rhs.name);
    return compare(theLHS, theRHS);
  }
  
  //--------------------------------------------------------------
  
  Filters::Filters()  {}
  
  Filters::Filters(const Filters &aCopy) {
      for (const auto& exp : aCopy.expressions)
          expressions.push_back(std::make_unique<Expression>(*exp));
  }
  
  Filters::~Filters() {
    //no need to delete expressions, they're unique_ptrs!
  }
  Filters& Filters::operator=(const Filters& aCopy) {
      for (const auto& exp : aCopy.expressions)
          expressions.push_back(std::make_unique<Expression>(*exp));
      return *this;
  }

  Filters& Filters::add(Expression *anExpression) {
    expressions.emplace_back(std::unique_ptr<Expression>(anExpression));
    return *this;
  }
    
  //compare expressions to row; return true if matches
  bool Filters::matches(const RowKeyValues &aList) const {
    bool theResult = true;
    for(auto &theExpr : expressions) {
        LogicOp theOp = logicalOps[theExpr->logic];
        theResult = theOp(theResult, (*theExpr)(aList));
    }
    return theResult;
   }
 

  //where operand is field, number, string...
  StatusResult parseOperand(CommandTokenReader &aTokenizer,
                            Schema &aSchema, Operand &anOperand) {
    StatusResult theResult{};
    Token &theToken = aTokenizer.current();
    if(TokenType::identifier==theToken.type) {
      auto theAttr = aSchema.getAttribute(theToken.data);
      if(theAttr.isValid()) {
        anOperand.ttype=theToken.type;
        anOperand.name=theToken.data; //hang on to name...
        anOperand.schemaId=aSchema.getHash(theToken.data);
        anOperand.dtype=theAttr.getType();
      }
      else {
        anOperand.ttype=TokenType::string;
        anOperand.dtype=DataTypes::varchar_type;
        anOperand.value=theToken.data;
      }
    }
    else if(TokenType::number==theToken.type) {
      anOperand.ttype=TokenType::number;
      anOperand.dtype=DataTypes::int_type;
      if (theToken.data.find('.')!=std::string::npos) {
        anOperand.dtype=DataTypes::float_type;
        anOperand.value=std::stof(theToken.data);
      }
      else anOperand.value=std::stoi(theToken.data);
    }
    else theResult.error=Errors::syntaxError;
    if(theResult) aTokenizer.next();
    return theResult;
  }
    
  //STUDENT: Add validation here...
  bool validateOperands(Operand &aLHS, Operand &aRHS, Schema &aSchema) {
    if(TokenType::identifier==aLHS.ttype) { //most common case...
      //STUDENT: Add code for validation as necessary
        if (!aSchema.checkContains(StringList({ aLHS.name }))) return false;
    }
    if(TokenType::identifier==aRHS.ttype) {
        if (!aSchema.checkContains(StringList({ aRHS.name }))) return false;
    }
    return true;
  }

  bool isValidOperand(Token &aToken) {
    //identifier, number, string...
    if(aToken.type==TokenType::identifier) return true;
    if (aToken.type == TokenType::number) return true;
    if (aToken.type == TokenType::string) return true;
    if (aToken.type == TokenType::timedate) return true;
    return false;
  }
  bool isLogicalOp(Token& aToken) {
      //identifier, number, string...
      if (aToken.keyword == Keywords::and_kw) return true;
      if (aToken.keyword == Keywords::not_kw) return true;
      if (aToken.keyword == Keywords::or_kw) return true;
      return false;
  }

  //STUDENT: This starting point code may need adaptation...
  StatusResult Filters::parse(CommandTokenReader &aTokenizer,Schema &aSchema) {
    StatusResult  theResult;
    ParseHelper theHelper(aTokenizer);
    while(theResult && (2<aTokenizer.remaining())) {
      Logical theLogic;
      bool hasLogical;
      if((hasLogical = isLogicalOp(aTokenizer.current()))) theHelper.parseLogical(theLogic);
      if(isValidOperand(aTokenizer.current())) {
        Expression theExpr;
        if((theResult=theHelper.parseExpression(aSchema,theExpr))) {
            if (hasLogical) theExpr.logic = theLogic;
            expressions.emplace_back(std::make_unique<Expression>(theExpr));
        }
      }
      
      else break;
    }
    return theResult;
  }

}


