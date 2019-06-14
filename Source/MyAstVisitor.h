#ifndef __NAMELINT_MY_AST_VISITOR__H__
#define __NAMELINT_MY_AST_VISITOR__H__

#include <iostream>
#include <vector>

#include <llvm/Support/CommandLine.h>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/Parser.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "Config.h"
#include "Detection.h"
#include "MyAstConsumer.h"
#include "TraceMemo.h"

using namespace std;
using namespace clang;
using namespace clang::tooling;
using namespace llvm;
using namespace clang::driver;
using namespace clang::tooling;
using namespace namelint;

class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
  private:
    ASTContext *m_pAstCxt;
    const SourceManager *m_pSrcMgr;
    namelint::Detection m_Detect;
    string m_FileName;

    bool m_bCheckFile;
    bool m_bCheckFunction;
    bool m_bCheckVariable;
    bool bAllowedEndWithUnderscope;

    namelint::RULETYPE m_FileRuleType;
    namelint::RULETYPE m_FunctionRuleType;
    namelint::RULETYPE m_VariableRuleType;

    map<string, string> m_HungarianList;
    map<string, string> m_HungarianListEx;
    vector<string> m_FileExt;
    vector<string> m_IgnoredFuncName;
    vector<string> m_IgnoredFuncPrefix;
    vector<string> m_IgnoredVarPrefix;

    bool _IsMainFile(Decl *pDecl);
    bool _PrintPosition(Decl *pDecl);
    void _KeepFileName(string &Path);
    bool _GetPosition(Decl *pDecl, string &FileName, size_t &nLineNumb,
                      size_t &nColNumb);

    bool _ClassifyTypeName(string &TyeName);

    ErrorDetail *_CreateErrorDetail(Decl *pDecl, const CheckType &CheckType,
                                    const string &TargetName,
                                    const string &Expected);

    ErrorDetail *_CreateErrorDetail(Decl *pDecl, const CheckType &CheckType,
                                    const string &TypeName,
                                    const string &TargetName,
                                    const string &Suggestion);

    bool _GetFunctionInfo(FunctionDecl *pDecl, string &FuncName);
    bool _GetParmsInfo(ParmVarDecl *pDecl, string &VarType, string &VarName);
    bool _GetVarInfo(VarDecl *pDecl, string &VarType, string &VarName);

  public:
    MyASTVisitor(const SourceManager *pSM, const ASTContext *pAstCxt,
                 const namelint::Config *pConfig);
    // bool VisitStmt(Stmt *pStmt);
    // bool VisitCXXRecordDecl(CXXRecordDecl *D);
    // bool VisitCXXConstructorDecl(CXXConstructorDecl *D);
    bool VisitFunctionDecl(FunctionDecl *pDecl);
    bool VisitCXXMethodDecl(CXXMethodDecl *pDecl);
    bool VisitRecordDecl(RecordDecl *pDecl);
    bool VisitVarDecl(VarDecl *pDecl);
    bool VisitReturnStmt(ReturnStmt *pRetStmt);
};

#endif // __NAMELINT_MY_AST_VISITOR__H__