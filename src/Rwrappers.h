/* $Id: Rwrappers.hpp 325 2016-08-22 18:12:16Z pjohnson $
  ---------------------------------------------------------------------------
  Simple C++ wrappers around R datatypes
  ---------------------------------------------------------------------------
  Copyright (C) 2011 Philip Johnson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  ---------------------------------------------------------------------------
*/
#ifndef RWRAPPERS_HPP
#define RWRAPPERS_HPP

#define R_NO_REMAP
#include <R.h>
#include <Rdefines.h>

/*---------------------------------------------------------------------------*/
// Note the R concept of a Vector is called "numeric", "character" or
// "real" in the C code.
template<typename Tlogic, typename Timpl = Tlogic> class CRVectorBase {
protected:
    typedef Timpl (*TGetter) (SEXP, R_xlen_t);
    typedef void (*TSetter) (SEXP, R_xlen_t, Timpl);

    CRVectorBase(SEXP v, TGetter getter, TSetter setter) {
        PROTECT(v);
        m_N = Rf_length(v);
        m_Sexp = v;
        m_Getter = getter;
        m_Setter = setter;
        UNPROTECT(1);
    }
public:
    int GetN(void) const {return m_N;}
    unsigned int size(void) const {return m_N;}
    const char* GetLevel(int i) const {
        --i;
        SEXP levels = Rf_getAttrib(m_Sexp, R_LevelsSymbol);
        if (!Rf_isNull(levels)  &&  i < Rf_length(levels)) {
            return CHAR(STRING_ELT(levels, i));
        } else {
            return "";
        }
    }
    int GetLevel(const char* s) const {
        SEXP levels = Rf_getAttrib(m_Sexp, R_LevelsSymbol);
        if (!Rf_isNull(levels)) {
            for (int i = 0;  i < Rf_length(levels);  ++i) {
                if (strcmp(CHAR(STRING_ELT(levels, i)), s) == 0) {
                    return i+1;
                }
            }
        }
        return 0;
    }
    const Timpl operator[] (int i) const {
        if (i >= m_N) { Rf_error("CRVector[] out of bounds"); }
        return m_Getter(m_Sexp, i);
    }

    // setters
    void Set(int i, const Timpl &v) { m_Setter(m_Sexp, i, v); }
    const char* GetName(int i) const {
        if (i >= m_N) { Rf_error("CRVector::GetName out of bounds"); }
        SEXP names = Rf_getAttrib(m_Sexp, R_NamesSymbol);
        if (Rf_isNull(names)) {
            return "";
        }
        return CHAR(STRING_ELT(names, i));
    }
    void SetName(int i, const char* name) {
        if (i >= m_N) { Rf_error("CRVector::SetName out of bounds"); }
        SEXP names = Rf_getAttrib(m_Sexp, R_NamesSymbol);
        if (Rf_isNull(names)) {
            names = Rf_allocVector(STRSXP, m_N);
            Rf_setAttrib(m_Sexp, R_NamesSymbol, names);
        }
        SET_STRING_ELT(names, i, Rf_mkChar(name));
    }
    operator SEXP() { return m_Sexp; }
protected:
    SEXP m_Sexp;
    TGetter m_Getter;
    TSetter m_Setter;
    int m_N;
};

template <typename T> class CRVector {};
template<> class CRVector<double> : public CRVectorBase<double> {
public:
    CRVector(SEXP p) : CRVectorBase<double>(p, REAL_ELT, SET_REAL_ELT) {}
    CRVector(int n) :
        CRVectorBase<double>(Rf_allocVector(REALSXP, n),
                             REAL_ELT, SET_REAL_ELT) {}
};
template<> class CRVector<int> : public CRVectorBase<int> {
public:
    CRVector(SEXP p) : CRVectorBase<int>(p, INTEGER_ELT, SET_INTEGER_ELT) {}
    CRVector(int n) :
        CRVectorBase<int>(Rf_allocVector(INTSXP, n),
                          INTEGER_ELT, SET_INTEGER_ELT) {}
};
template<> class CRVector<bool> : public CRVectorBase<bool,int> {
public:
    CRVector(SEXP p) :
        CRVectorBase<bool,int>(p, LOGICAL_ELT, SET_LOGICAL_ELT) {}
    CRVector(int n) :
        CRVectorBase<bool,int>(Rf_allocVector(LGLSXP, n),
                               LOGICAL_ELT, SET_LOGICAL_ELT) {}
};
template<> class CRVector<const char*> : public CRVectorBase<SEXP> {
public:
    CRVector(SEXP p) :
        CRVectorBase<SEXP>(p, STRING_ELT, SET_STRING_ELT) {}
    CRVector(int n) :
        CRVectorBase<SEXP>(Rf_allocVector(STRSXP, n), STRING_ELT, SET_STRING_ELT) {}
    const char* operator[] (int i) const {
        if (i >= m_N) { Rf_error("CRVector[] out of bounds"); }
        return CHAR(STRING_ELT(m_Sexp, i));
    }
    void Set(int i, const char *s) {
        if (i >= m_N) { Rf_error("CRVector[] out of bounds"); }
        SET_STRING_ELT(m_Sexp, i, Rf_mkChar(s));
    }
};


/*---------------------------------------------------------------------------*/
class CRSEXP {
public:
    CRSEXP(SEXP s) { m_SEXP = s; }
    operator SEXP() {return m_SEXP;}
    CRVector<double> AsDouble(void) { return m_SEXP;}
    CRVector<int> AsInt(void) { return m_SEXP;}
    SEXP m_SEXP;
};

/*---------------------------------------------------------------------------*/
// For unclear reasons, the R datatype "List" is called "Vector" in
// the underlying C code.
class CRList {
public:
    CRList(SEXP l) {
        if (!Rf_isVector(l)) {
            Rf_error("logic flaw: CRList constructed on non-list SEXP.");
        }
        m_N = Rf_length(l);
        m_Names = (Rf_isNull(Rf_getAttrib(l, R_NamesSymbol))) ? NULL :
            new CRVector<const char*>(Rf_getAttrib(l, R_NamesSymbol));
        m_Sexp = l;
    }
    CRList(int n) { //create new list
        m_N = n;
        m_Names = NULL;
        m_Sexp = Rf_allocVector(VECSXP, m_N);
    }
    ~CRList() {
        delete m_Names;
    }
    unsigned int size(void) const { return m_N; }
    operator SEXP() { return m_Sexp; }

    bool HasEntry(const char* target) const {
        int i;
        for (i = 0;  i < m_N  &&  strcmp((*m_Names)[i], target) != 0;  ++i);
        return i < m_N;
    }
    CRSEXP operator[] (int i) const {
        if (i >= m_N) {
            Rf_error("CRList[] out of bounds (requested %d)", i);
        }
        return VECTOR_ELT(m_Sexp, i);
    }
    CRSEXP operator[] (const char* target) const {
        int i;
        for (i = 0;  i < m_N;  ++i) {
            if (strcmp((*m_Names)[i], target) == 0) {
                return VECTOR_ELT(m_Sexp, i);
            }
        }
        Rf_error("could not find list element named '%s'", target);
        return SEXP();//just to silence compiler
    }

    // setters
    void SetSEXP(int i, SEXP p, const char* name=NULL) {
        if (i >= m_N) {
            Rf_error("Logic flaw: tried to set element off end of CRList");
        }
        if (name) {
            if (!m_Names) {
                m_Names = new CRVector<const char*>(m_N);
                Rf_setAttrib(m_Sexp, R_NamesSymbol, *m_Names);
            }
            m_Names->Set(i, name);
        }
        SET_VECTOR_ELT(m_Sexp, i, p);
    }
    

private:
    SEXP m_Sexp;
    CRVector<const char*> *m_Names;
    int m_N;
};

/*---------------------------------------------------------------------------*/

template<typename T> class CRMatrixBase {
protected:
    //data must correspond to SEXP!
    CRMatrixBase(SEXP m, T* fconverter (SEXP x)) {
        m_NumRows = INTEGER(Rf_getAttrib(m, R_DimSymbol))[0];
        m_NumCols = INTEGER(Rf_getAttrib(m, R_DimSymbol))[1];
        m_Data = (T*) fconverter(m);
        m_Sexp = m;
    }
public:
    int nrow(void) const {return m_NumRows;}
    int ncol(void) const {return m_NumCols;}
    int GetNumRows(void) const {return m_NumRows;}
    int GetNumCols(void) const {return m_NumCols;}
    //note R uses column-wise data storage
    const T& operator() (int i, int j) const {return m_Data[j*m_NumRows + i]; }
    T& operator() (int i, int j) {return m_Data[j*m_NumRows + i]; }

    void SetColName(int j, const char* name) {
        if (Rf_isNull(Rf_getAttrib(m_Sexp, R_DimNamesSymbol))) {
            CRList dims(2);
            Rf_setAttrib(m_Sexp, R_DimNamesSymbol, dims);
        }
        if (Rf_isNull(VECTOR_ELT(Rf_getAttrib(m_Sexp, R_DimNamesSymbol), 1))) {
            //CRVector<const char*> colnames(m_NumCols);
            SEXP colnames = Rf_allocVector(STRSXP, m_NumCols);
            SET_VECTOR_ELT(Rf_getAttrib(m_Sexp, R_DimNamesSymbol), 1, colnames);
        }
        SET_STRING_ELT(VECTOR_ELT(Rf_getAttrib(m_Sexp, R_DimNamesSymbol), 1),
                       j, Rf_mkChar(name));
        /*
        CRList colnames(VECTOR_ELT(GET_DIMNAMES(m_Sexp), 1));
        colnames.SetSEXP(j, Rf_mkChar(name));
        */
    }
    operator SEXP() { return m_Sexp; }
private:
    SEXP m_Sexp;
    T *m_Data;
    int m_NumRows;
    int m_NumCols;
};
template <typename T> class CRMatrix {};
template<> class CRMatrix<double> : public CRMatrixBase<double> {
public:
    CRMatrix(SEXP p) : CRMatrixBase<double>(p, REAL) {}
    CRMatrix(int r, int c) :
        CRMatrixBase<double>(Rf_allocMatrix(REALSXP, r, c), REAL) {}
};
template<> class CRMatrix<int> : public CRMatrixBase<int> {
public:
    CRMatrix(SEXP p) : CRMatrixBase<int>(p, INTEGER) {}
    CRMatrix(int r, int c) :
        CRMatrixBase<int>(Rf_allocMatrix(INTSXP, r, c), INTEGER) {}
};

#endif
