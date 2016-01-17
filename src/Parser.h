#ifndef _PARSER_H_
#define _PARSER_H_

#include <cassert>
#include <stack>
#include <memory>
#include "ast.h"
#include "expr.h"
#include "decl.h"
#include "lexer.h"
#include "symbol.h"
#include "error.h"

class Env;

typedef std::pair<const char*, Type*> NameTypePair;

class Parser
{
public:
    explicit Parser(Lexer* lexer): _lexer(lexer) {}

    ~Parser(void) { 
        delete _lexer;
    }

	Expr* ParseConstant(const Token* tok);
	Expr* ParseString(const Token* tok);
	Expr* ParseGeneric(void);

	TranslationUnit* ParseTranslationUnit(void);

    /****** Declaration ******/

	Decl* ParseFuncDef(void);
	Type* ParseDecl(std::list<Expr*> initializers);

    /************ Expressions ************/
	
	Expr* ParseExpr(void);

	Expr* ParsePrimaryExpr(void);

	Expr* ParsePostfixExpr(void);
	Expr* ParsePostfixExprTail(Expr* primExpr);
	Expr* ParseSubScripting(Expr* pointer);
	Expr* ParseMemberRef(int tag, Expr* lhs);
	UnaryOp* ParsePostfixIncDec(int tag, Expr* expr);
	FuncCall* ParseFuncCall(Expr* caller);

	Expr* ParseUnaryExpr(void);
	Constant* ParseSizeof(void);
	Constant* ParseAlignof(void);
	UnaryOp* ParsePrefixIncDec(int tag);
	UnaryOp* ParseUnaryOp(int op);
	//UnaryOp* ParseDerefOperand(void);

	Type* ParseTypeName(void);
	Expr* ParseCastExpr(void);
	Expr* ParseMultiplicativeExpr(void);
	Expr* ParseAdditiveExpr(void);
	Expr* ParseShiftExpr(void);
	Expr* ParseRelationalExpr(void);
	Expr* ParseEqualityExpr(void);
	Expr* ParseBitiwiseAndExpr(void);
	Expr* ParseBitwiseXorExpr(void);
	Expr* ParseBitwiseOrExpr(void);
	Expr* ParseLogicalAndExpr(void);
	Expr* ParseLogicalOrExpr(void);
	Expr* ParseConditionalExpr(void);

	Expr* ParseCommaExpr(void);

	Expr* ParseAssignExpr(void);

	Constant* ParseConstantExpr(void);


	/************* Declarations **************/
	void ParseDecl(std::list<Expr*>& initializers);
	Type* ParseDeclSpec(int* storage, int* func);
	Type* ParseSpecQual(void);
	int ParseAlignas(void);
	Type* ParseStructUnionSpec(bool isStruct);
	StructUnionType* ParseStructDecl(StructUnionType* type);
	
	//declarator
	int ParseQual(void);
	Type* ParsePointer(Type* typePointedTo);
	Variable* ParseDeclaratorAndDo(Type* base, int storageSpec, int funcSpec);
	NameTypePair ParseDeclarator(Type* type);
	Type* ParseArrayFuncDeclarator(Type* base);
	int ParseArrayLength(void);
	bool ParseParamList(std::list<Type*>& params);
	Type* ParseParamDecl(void);

	//typename
	Type* ParseAbstractDeclarator(Type* type);

	//initializer
	Expr* ParseInitDeclarator(Type* type, int storageSpec, int funcSpec);
	//Expr* ParseInit

	/************* Statements ***************/



private:
	//�����ǰtoken���ϲ���������true,��consumeһ��token
	//�����tokTag�������򷵻�false�����Ҳ�consume token
	bool Try(int tokTag) {
		auto tok = Next();
		if (tok->Tag() == tokTag)
			return true;
		PutBack();
		return false;
	}

	//���ص�ǰtoken����ǰ��
	Token* Next(void) { return _lexer->Get(); }
	void PutBack(void) { _lexer->Unget(); }

	//���ص�ǰtoken�����ǲ�ǰ��
	Token* Peek(void) { return _lexer->Peek(); }
	const Token* Peek(void) const{ return _lexer->Peek(); }
	bool Test(int tag) const { return Peek()->Tag() == tag; }

	//��¼��ǰtokenλ��
	void Mark(void) { _buf.push(Peek()); }

	//�ص����һ��Mark�ĵط�
	Token* Release(void) {
		assert(!_buf.empty());
		while (Peek() != _buf.top()) {
			PutBack();
		}
		_buf.pop();
		return Peek();
	}

	bool IsTypeName(const Token* tok) {
		if (tok->IsTypeQual() || tok->IsTypeSpec())
			return true;
		return (tok->IsIdentifier() 
			&& nullptr != _topEnv->FindType(tok->Val()));
	}

	void Expect(int expect, int follow1 = ',', int follow2 = ';');

	void Panic(int follow1, int follow2) {
		for (const Token* tok = Next(); !tok->IsEOF(); tok = Next()) {
			if (tok->Tag() == follow1 || tok->Tag() == follow2)
				return PutBack();
		}
	}

	void EnsureModifiable(Expr* expr) const {
		if (!expr->IsLVal()) {
			//TODO: error
			Error("lvalue expression expected");
		} else if (expr->Ty()->IsConst()) {
			Error("can't modifiy 'const' qualified expression");
		}
	}

	void EnsureLVal(Expr* expr) const {
		if (!expr->IsLVal()) {
			Error("lvalue expected");
		}
	}

	void EnterBlock(void) {
		_topEnv = new Env(_topEnv);
	}
	void ExitBlock(void) {
		_topEnv = _topEnv->Parent();
	}

private:
    Lexer* _lexer;
	Env* _topEnv;
	std::stack<Token*> _buf;
};

#endif