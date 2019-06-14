#include "MyAstVisitor.h"
#include "Common.h"
#include <iomanip>

using namespace namelint;

bool MyASTVisitor::_IsMainFile(Decl *pDecl) {
    if (this->m_pAstCxt->getSourceManager().isInMainFile(
            pDecl->getLocation())) {
        return true;
    }
    return false;
}

void MyASTVisitor::_KeepFileName(string &FilePath) {
    size_t nPos = FilePath.rfind("\\");
    if (nPos < 0) {
        nPos = FilePath.rfind("/");
    }

    if (nPos > 0) {
        FilePath = FilePath.substr(nPos + 1, FilePath.length() - nPos - 1);
    }
}

bool MyASTVisitor::_GetPosition(Decl *pDecl, string &FileName,
                                size_t &nLineNumb, size_t &nColNumb) {
    if (!this->m_pAstCxt) {
        this->m_pAstCxt = &pDecl->getASTContext();
        assert(false);
        return false;
    }

    bool bStatus = false;

    FullSourceLoc FullLocation =
        this->m_pAstCxt->getFullLoc(pDecl->getBeginLoc());
    if (FullLocation.isValid()) {
        FileName = FullLocation.getFileLoc().getFileEntry()->getName();

        if ((FileName != GetAppCxt()->FileName) ||
            ("" == GetAppCxt()->FileName)) {
            APP_CONTEXT *pAppCxt = (APP_CONTEXT *)GetAppCxt();
            pAppCxt->FileName    = FileName;

            string Path1;
            Path::NormPath(FileName.c_str(), Path1);

            String::Replace(Path1, "\\\\", "\\");
            String::Replace(Path1, "\"", "");
            FileName = Path1;
        }

        nLineNumb = FullLocation.getSpellingLineNumber();
        nColNumb  = FullLocation.getSpellingColumnNumber();
        bStatus   = true;
    }

    return bStatus;
}

bool MyASTVisitor::_PrintPosition(Decl *pDecl) {
    string FileName;
    size_t nLineNumb = 0;
    size_t nColNumb  = 0;
    bool bStatus     = _GetPosition(pDecl, FileName, nLineNumb, nColNumb);
    if (bStatus) {
        cout << "  <" << nLineNumb << "," << nColNumb << ">" << setw(12);
    }
    return bStatus;
}

bool MyASTVisitor::_ClassifyTypeName(string &TyeName) {
    bool bStatus = true;

    String::Replace(TyeName, "extern", "");
    String::Replace(TyeName, "static", "");
    String::Replace(TyeName, "struct", "");
    String::Replace(TyeName, "const", "");
    String::Replace(TyeName, "&", "");
    String::Replace(TyeName, "* ", "*");
    String::Replace(TyeName, " *", "*");
    String::Trim(TyeName);

    return bStatus;
}

bool MyASTVisitor::_GetFunctionInfo(FunctionDecl *pDecl, string &Name) {
    if (!pDecl->hasBody()) {
        return false;
    }
    if (!this->_IsMainFile(pDecl)) {
        return false;
    }

    Name = pDecl->getDeclName().getAsString();
    return true;
}

bool MyASTVisitor::_GetParmsInfo(ParmVarDecl *pDecl, string &VarType,
                                 string &VarName) {
    if (!pDecl) {
        return false;
    }
    if (!this->_IsMainFile(pDecl)) {
        return false;
    }

    bool bInvalid = false;
    clang::StringRef MyStrRef;
    clang::LangOptions MyLangOpt;
    SourceRange MySrcRange(pDecl->getBeginLoc(), pDecl->getLocation());
    MyStrRef = Lexer::getSourceText(CharSourceRange::getCharRange(MySrcRange),
                                    *this->m_pSrcMgr, MyLangOpt, &bInvalid);
    VarType  = MyStrRef.str();

    QualType QualType = pDecl->getType();

    VarName = pDecl->getName().data();
    // VarType = QualType.getAsString();
    if (VarType.length() > 0) {
        this->_ClassifyTypeName(VarType);
    }

    return true;
}

bool MyASTVisitor::_GetVarInfo(VarDecl *pDecl, string &VarType,
                               string &VarName) {
    if (!pDecl) {
        return false;
    }

    if (!this->_IsMainFile(pDecl)) {
        return false;
    }

    bool bInvalid = false;
    clang::StringRef MyStrRef;
    clang::LangOptions MyLangOpt;
    SourceLocation MyBeginLoc = pDecl->getBeginLoc();
    SourceLocation MyLoc      = pDecl->getLocation();
    SourceRange MySrcRange(MyBeginLoc, MyLoc);
    MyStrRef = Lexer::getSourceText(CharSourceRange::getCharRange(MySrcRange),
                                    *this->m_pSrcMgr, MyLangOpt, &bInvalid);

    string RawSrcText   = MyStrRef.str();
    QualType myQualType = pDecl->getType();
    VarName             = pDecl->getNameAsString();
    if (RawSrcText.length() > 0) {
        this->_ClassifyTypeName(RawSrcText);
    }
    VarType = RawSrcText;

    return true;
}

ErrorDetail *MyASTVisitor::_CreateErrorDetail(Decl *pDecl,
                                              const CheckType &CheckType,
                                              const string &TargetName,
                                              const string &Suggestion) {
    return this->_CreateErrorDetail(pDecl, CheckType, "", TargetName,
                                    Suggestion);
}

ErrorDetail *MyASTVisitor::_CreateErrorDetail(Decl *pDecl,
                                              const CheckType &CheckType,
                                              const string &TypeName,
                                              const string &TargetName,
                                              const string &Suggestion) {
    if (!pDecl) {
        return NULL;
    }

    ErrorDetail *pNew = NULL;

    string FileName;
    CodePos Pos = {0};
    if (this->_GetPosition(pDecl, FileName, Pos.nLine, Pos.nColumn)) {
        pNew =
            new ErrorDetail(Pos, CheckType, TypeName, TargetName, Suggestion);
    }
    return pNew;
}

MyASTVisitor::MyASTVisitor(const SourceManager *pSM, const ASTContext *pAstCxt,
                           const namelint::Config *pConfig) {
    this->m_pSrcMgr = pSM;
    this->m_pAstCxt = (ASTContext *)pAstCxt;

    const ConfigData &CfgData = pConfig->GetData();

    this->m_FileExt        = CfgData.m_General.FileExtName;
    this->m_bCheckFile     = CfgData.m_General.bCheckFileName;
    this->m_bCheckFunction = CfgData.m_General.bCheckFunctionName;
    this->m_bCheckVariable = CfgData.m_General.bCheckVariableName;

    this->m_FileRuleType     = CfgData.m_Rule.FileName;
    this->m_FunctionRuleType = CfgData.m_Rule.FunctionName;
    this->m_VariableRuleType = CfgData.m_Rule.VariableName;

    this->m_IgnoredFuncName   = CfgData.m_WhiteList.IgnoredFuncName;
    this->m_IgnoredFuncPrefix = CfgData.m_WhiteList.IgnoredFuncPrefix;
    this->m_IgnoredVarPrefix  = CfgData.m_WhiteList.VariablePrefix;

    this->m_HungarianList   = CfgData.m_HungarianList.MappedTable;
    this->m_HungarianListEx = CfgData.m_HungarianListEx.MappedTable;

    this->bAllowedEndWithUnderscope =
        CfgData.m_WhiteList.bAllowedEndWithUnderscope;
}

bool MyASTVisitor::VisitFunctionDecl(clang::FunctionDecl *pDecl) {
    if (!this->m_bCheckFunction) {
        return true;
    }

    APP_CONTEXT *pAppCxt = ((APP_CONTEXT *)GetAppCxt());
    if (!pAppCxt) {
        assert(pAppCxt);
        return false;
    }

    string FuncName;
    bool bResult = false;
    bool bStatus = this->_GetFunctionInfo(pDecl, FuncName);
    if (bStatus) {
        bResult = this->m_Detect.CheckFunction(
            this->m_FunctionRuleType, FuncName, this->m_IgnoredFuncName,
            this->m_IgnoredFuncPrefix, this->bAllowedEndWithUnderscope);

        pAppCxt->TraceMemo.Checked.nFunction++;
        if (!bResult) {
            pAppCxt->TraceMemo.Error.nFunction++;

            pAppCxt->TraceMemo.ErrorDetailList.push_back(
                this->_CreateErrorDetail(pDecl, CheckType::CT_Function,
                                         FuncName, ""));
        }

        const clang::ArrayRef<clang::ParmVarDecl *> MyRefArray =
            pDecl->parameters();
        for (size_t nIdx = 0; nIdx < MyRefArray.size(); nIdx++) {
            string VarType;
            string VarName;
            ParmVarDecl *pParmVarDecl = MyRefArray[nIdx];

            bStatus = this->_GetParmsInfo(pParmVarDecl, VarType, VarName);
            if (bStatus) {
                bResult = this->m_Detect.CheckVariable(
                    this->m_VariableRuleType, VarType, VarName,
                    this->m_IgnoredVarPrefix, this->m_HungarianList,
                    this->m_HungarianListEx);

                pAppCxt->TraceMemo.Checked.nParameter++;
                if (!bResult) {
                    pAppCxt->TraceMemo.Error.nParameter++;

                    pAppCxt->TraceMemo.ErrorDetailList.push_back(
                        this->_CreateErrorDetail(pParmVarDecl,
                                                 CheckType::CT_Parameter,
                                                 VarType, VarName, ""));
                }
            }
        }
    }

    return bStatus;
}

bool MyASTVisitor::VisitCXXMethodDecl(CXXMethodDecl *pDecl) { return true; }

bool MyASTVisitor::VisitRecordDecl(RecordDecl *pDecl) {
    if (!this->_IsMainFile(pDecl)) {
        return false;
    }

    return true;
}

bool MyASTVisitor::VisitVarDecl(VarDecl *pDecl) {
    if (!this->m_bCheckVariable) {
        return true;
    }

    APP_CONTEXT *pAppCxt = ((APP_CONTEXT *)GetAppCxt());
    if (!pAppCxt) {
        assert(pAppCxt);
        return false;
    }

    string VarType;
    string VarName;

    bool bStauts = this->_GetVarInfo(pDecl, VarType, VarName);
    if (bStauts) {
        bool bResult = this->m_Detect.CheckVariable(
            this->m_VariableRuleType, VarType, VarName,
            this->m_IgnoredVarPrefix, this->m_HungarianList,
            this->m_HungarianListEx);

        pAppCxt->TraceMemo.Checked.nVariable++;
        if (!bResult) {
            pAppCxt->TraceMemo.Error.nParameter++;

            pAppCxt->TraceMemo.ErrorDetailList.push_back(
                this->_CreateErrorDetail(pDecl, CheckType::CT_Variable, VarType,
                                         VarName, ""));
        }
    }

    return bStauts;
}

bool MyASTVisitor::VisitReturnStmt(ReturnStmt *pRetStmt) {
    assert(pRetStmt);

    const Expr *pExpr = pRetStmt->getRetValue();
    if (pExpr) {
        clang::QualType MyQualType = pExpr->getType();
        std::string MyTypeStr      = MyQualType.getAsString();
        return true;
    }

    return false;
}