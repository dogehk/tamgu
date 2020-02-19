/*
 *  Tamgu (탐구)
 *
 * Copyright 2019-present NAVER Corp.
 * under BSD 3-clause
 */
/* --- CONTENTS ---
 Project    : Tamgu (탐구)
 Version    : See tamgu.cxx for the version number
 filename   : tamgudvector.cxx
 Date       : 2017/09/01
 Purpose    : vector implementation
 Programmer : Claude ROUX (claude.roux@naverlabs.com)
 Reviewer   :
*/

#include "tamgu.h"
#include "tamgutaskell.h"
#include "tamgudvector.h"
#include "tamgufvector.h"
#include "tamguivector.h"
#include "tamgudecimal.h"
#include "tamguconstants.h"
#include "tamgumap.h"
#include "tamguvector.h"
#include "constobjects.h"
#include "instructions.h"

//We need to declare once again our local definitions.
Exporting basebin_hash<dvectorMethod>  Tamgudvector::methods;
Exporting hmap<string, string> Tamgudvector::infomethods;
Exporting bin_hash<unsigned long> Tamgudvector::exported;

Exporting short Tamgudvector::idtype = 0;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
    #define min(a,b)            (((a) < (b)) ? (a) : (b))
        #endif

//MethodInitialization will add the right references to "name", which is always a new method associated to the object we are creating
void Tamgudvector::AddMethod(TamguGlobal* global, string name, dvectorMethod func, unsigned long arity, string infos) {
    short idname = global->Getid(name);
    methods[idname] = func;
    infomethods[name] = infos;
    exported[idname] = arity;
}



    void Tamgudvector::Setidtype(TamguGlobal* global) {
        Tamgudvector::idtype = global->Getid("dvector");
    }

   bool Tamgudvector::InitialisationModule(TamguGlobal* global, string version) {
    methods.clear();
    infomethods.clear();
    exported.clear();



    Tamgudvector::idtype = global->Getid("dvector");

    Tamgudvector::AddMethod(global, "reverse", &Tamgudvector::MethodReverse, P_NONE, "reverse(): reverse a vector.");
    Tamgudvector::AddMethod(global, "reserve", &Tamgudvector::MethodReserve, P_ONE, "reserve(int sz): Reserve a size of 'sz' potential element in the vector.");
    Tamgudvector::AddMethod(global, "unique", &Tamgudvector::MethodUnique, P_NONE, "unique(): remove duplicate elements.");

    Tamgudvector::AddMethod(global, "remove", &Tamgudvector::MethodRemove, P_ONE, "remove(decimal e): remove 'e' from the vector.");

    Tamgudvector::AddMethod(global, "last", &Tamgudvector::MethodLast, P_NONE, "last(): return the last element.");
    Tamgudvector::AddMethod(global, "join", &Tamgudvector::MethodJoin, P_ONE, "join(string sep): Produce a string representation for the container.");

    Tamgudvector::AddMethod(global, "shuffle", &Tamgudvector::MethodShuffle, P_NONE, "shuffle(): shuffle the values in the vector.");
    Tamgudvector::AddMethod(global, "sort", &Tamgudvector::MethodSort, P_ONE, "sort(bool reverse): sort the elements within.");
    Tamgudvector::AddMethod(global, "sum", &Tamgudvector::MethodSum, P_NONE, "sum(): return the sum of elements.");

    Tamgudvector::AddMethod(global, "product", &Tamgudvector::MethodProduct, P_NONE, "product(): return the product of elements.");
    Tamgudvector::AddMethod(global, "push", &Tamgudvector::MethodPush, P_ATLEASTONE, "push(v): Push a value into the vector.");
    Tamgudvector::AddMethod(global, "pop", &Tamgudvector::MethodPop, P_NONE | P_ONE, "pop(int i): Erase an element from the vector");
    Tamgudvector::AddMethod(global, "poplast", &Tamgudvector::MethodPoplast, P_NONE, "poplast(): remove and return the last element from the vector");
    Tamgudvector::AddMethod(global, "merge", &Tamgudvector::MethodMerge, P_ONE, "merge(v): Merge v into the vector.");
    Tamgudvector::AddMethod(global, "editdistance", &Tamgudvector::MethodEditDistance, P_ONE, "editdistance(v): Compute the edit distance with vector 'v'.");
    Tamgudvector::AddMethod(global, "insert", &Tamgudvector::MethodInsert, P_TWO, "insert(int i,v): Insert v at position i.");
    Tamgudvector::AddMethod(global, "permute", &Tamgudvector::MethodPermute, P_NONE, "permute(): permute the values in the vector after each call.");


    global->newInstance[Tamgudvector::idtype] = new Tamgudvector(global);
    global->RecordMethods(Tamgudvector::idtype,Tamgudvector::exported);

    return true;
}

Exporting TamguIteration* Tamgudvector::Newiteration(bool direction) {
    return new TamguIterationdvector(this, direction);
}


Exporting Tamgu* Tamgudvector::in(Tamgu* context, Tamgu* a, short idthread) {
    //Three cases along the container type...
    //It is a Boolean, we expect false or true
    //It is an integer, we expect a position in v
    //It is a container, we are looking for all positions...

    float val = a->Decimal();
     if (context->isVectorContainer()) {
        Tamguivector* v = (Tamguivector*)Selectaivector(context);
        Doublelocking _lock(this, v);

        for (size_t i = 0; i < values.size(); i++) {
            if (values[i] == val)
                v->values.push_back(i);
        }
        return v;
    }

   if (context->isNumber()) {
        Locking _lock(this);
        for (size_t i = 0; i < values.size(); i++) {
            if (values[i] == val)
                return globalTamgu->Provideint(i);
        }
        return aMINUSONE;
    }

    Locking _lock(this);
    for (size_t i = 0; i < values.size(); i++) {
        if (values[i] == val)
            return aTRUE;
    }

    return aFALSE;
}

Exporting Tamgu* Tamgudvector::getvalue(BLONG i) {
    Locking _lock(this);
    if (i<0 || i>=values.size())
        return aNOELEMENT;
    return new Tamgudecimal(values[i]);
}

Exporting Tamgu* Tamgudvector::Push(Tamgu* a) {
    Locking _lock(this);
    values.push_back(a->Decimal());
    return aTRUE;
}

Exporting Tamgu* Tamgudvector::Pop(Tamgu* idx) {
    Locking _lock(this);
    if (!values.size())
        return aFALSE;
        
    BLONG v = idx->Integer();
    if (v == -1) {
        v = values.size() - 1;
        values.pop_back();
    }
    else {
        if (v < 0 || v >= (BLONG)values.size())
            return aFALSE;
        values.erase(values.begin() + v);
    }
    return aTRUE;
}

Exporting void Tamgudvector::Clear() {
    //To set a variable back to empty
    Locking _lock(this);
    values.clear();
}



Exporting string Tamgudvector::String() {
    Locking _lock(this);
    size_t i;
    stringstream r;
    r<<"[";
    bool first=true;
    for (i=0;i<values.size();i++) {
        if (first)
            first=false;
        else
            r<<",";
        r<<values[i];
    }
    r<<"]";
    return r.str();
}


Exporting long Tamgudvector::Integer() {
    Locking _lock(this);
    return values.size();
}

Exporting float Tamgudvector::Decimal() {
    Locking _lock(this);
    return values.size();
}

Exporting double Tamgudvector::Float() {
    Locking _lock(this);
    return values.size();
}

Exporting BLONG Tamgudvector::Long() {
    Locking _lock(this);
    return values.size();
}

Exporting bool Tamgudvector::Boolean() {
    Locking _lock(this);
    if (values.size()==0)
        return false;
    return true;
}


//Basic operations
Exporting long Tamgudvector::Size() {
    Locking _lock(this);
    return values.size();
}

static bool compFloat(float s1, float s2) {
    if (s1<s2)
        return true;
    return false;
}

static bool icompFloat(float s1, float s2) {
    if (s1>s2)
        return true;
    return false;
}
Exporting void Tamgudvector::Sort(bool direction) {
    Locking _lock(this);
    if (direction == false)
        sort(values.begin(), values.end(), compFloat);
    else
        sort(values.begin(), values.end(), icompFloat);
}

Exporting Tamgu* Tamgudvector::Merging(Tamgu* ke) {
    if (!ke->isContainer()) {
        Locking _lock(this);
        values.push_back(ke->Decimal());
        return this;
    }

    if (ke->Size() == 0)
        return this;

    Doublelocking _lock(this, ke);
    //Three cases:
    if (ke->Type() == idtype) {
        Tamgudvector* kvect = (Tamgudvector*)ke;
        values.insert(values.end(), kvect->values.begin(), kvect->values.end());
        return this;
    }

    if (ke->Type() == a_ivector) {
        Tamguivector* kvect = (Tamguivector*)ke;
        values.insert(values.end(), kvect->values.begin(), kvect->values.end());
        return this;
    }

    TamguIteration* itr = (TamguIteration*)ke->Newiteration(false);
    for (itr->Begin(); itr->End() == aFALSE; itr->Next())
        values.push_back(itr->Valuedecimal());
    itr->Release();

    return this;
}

Exporting Tamgu* Tamgudvector::Combine(Tamgu* ke) {
    if (ke->Size() == 0)
        return this;
    
    long i;
    
    if (!ke->isContainer()) {
        float v=ke->Decimal();
        Locking _lock(this);
        for (i=0; i< values.size();i++)
            values[i]+=v;
        return this;
    }
    
    
    Doublelocking _lock(this, ke);
    i = 0;
    TamguIteration* itr = (TamguIteration*)ke->Newiteration(false);
    for (itr->Begin(); itr->End() == aFALSE; itr->Next()) {
        if (i == values.size()) {
            values.push_back(itr->Valuedecimal());
            i++;
            continue;
        }
        
        values[i] += itr->Valuedecimal();
        i++;
    }
    itr->Release();
    return this;
}

Exporting unsigned long Tamgudvector::EditDistance(Tamgu* e) {
    unsigned long s1len, s2len, x, y, lastdiag, olddiag;
    s1len = Size();
    s2len = e->Size();

    float* v1 = new float[s1len + 1];
    float* v2 = new float[s2len + 1];

    y = max(s1len, s2len);
    for (x = 0; x < y; x++) {
        if (x < s1len)
            v1[x] = values[x];
        if (x < s2len)
            v2[x] = e->getfloat(x);
    }

    size_t* column = new size_t[s1len + 1];
    for (y = 1; y <= s1len; y++)
        column[y] = y;

    unsigned int ed;
    for (x = 1; x <= s2len; x++) {
        column[0] = x;
        for (y = 1, lastdiag = x - 1; y <= s1len; y++) {
            olddiag = column[y];
            if (v1[y - 1] == v2[x - 1])
                ed = 0;
            else
                ed = 1;
            column[y] = MIN3(column[y] + 1, column[y - 1] + 1, lastdiag + ed);
            lastdiag = olddiag;
        }
    }

    delete[] v1;
    delete[] v2;

    s2len = column[s1len];
    delete[] column;
    return s2len;
}

Exporting Tamgu* Tamgudvector::Put(Tamgu* idx, Tamgu* ke, short idthread) {

    Doublelocking _lock(this, ke);

    if (!idx->isIndex()) {

        if (ke == this)
            return aTRUE;

        if (ke->isNULL()) {
            values.clear();
            return aTRUE;
        }

        if (ke->Type() == a_dvector) {
            values = ((Tamgudvector*)ke)->values;
            return aTRUE;
        }

        long sz = ke->Size();
        if (ke->isVectorContainer()) {
            values.clear();
            if (sz) {
                values.reserve(sz);
                for (long i=0; i< sz; i++)
                    values.push_back(ke->getdecimal(i));
            }
            return aTRUE;
        }

        if (ke->isMapContainer()) {
            values.clear();
            if (sz) {
                values.reserve(sz);
                TamguIteration* itr = (TamguIteration*)ke->Newiteration(false);
                for (itr->Begin(); itr->End() == aFALSE; itr->Next())
                    values.push_back(itr->Keyfloat());
                itr->Release();
            }
            return aTRUE;
        }

        if (ke->isContainer()) {
            values.clear();
            if (sz) {
                values.reserve(sz);
                TamguIteration* itr = (TamguIteration*)ke->Newiteration(false);
                for (itr->Begin(); itr->End() == aFALSE; itr->Next())
                    values.push_back(itr->Valuedecimal());
                itr->Release();
            }
            return aTRUE;
        }
        if (ke->isString()) {
            values.clear();
            string sv = ke->String();
            if (!v_comma_split_decimal(sv, values))
                return globalTamgu->Returnerror("Cannot set this value", idthread);
            return aTRUE;
        }
        ke = ke->Vector(idthread);
        if (!ke->isVectorContainer())
            return globalTamgu->Returnerror("Cannot set this value", idthread);

        sz = ke->Size();
        values.clear();
        if (sz) {
            values.reserve(sz);
            //We copy all values from ke to this
            for (long it = 0; it < ke->Size(); ++it)
                values.push_back(ke->getfloat(it));
        }
        ke->Release();
        return aTRUE;
    }
    else {
        //In this specific case, we try to replace a bloc of values with a new bloc
        if (idx->isInterval()) {
            //First we remove the values...
            long lkey = idx->Integer();
            if (lkey < 0)
                lkey = values.size() + lkey;

            long rkey = ((TamguIndex*)idx)->right->Integer();
            if (rkey < 0)
                rkey = values.size() + rkey;

            if (rkey < lkey || rkey >= values.size() || lkey >= values.size()) {
                if (globalTamgu->erroronkey)
                    globalTamgu->Returnerror("Wrong index", idthread);
                return aTRUE;
            }

            if (rkey != lkey)
                rkey--;

            while (rkey >= lkey) {
                values.erase(values.begin() + rkey);
                rkey--;
            }
            if (ke->isVectorContainer()) {
                for (long i = ke->Size() - 1; i >= 0; i--) {
                    values.insert(values.begin() + lkey, ke->getfloat(i));
                }
                return aTRUE;
            }

            if (ke->isContainer()) {
                TamguIteration* itr = (TamguIteration*)ke->Newiteration(false);
                for (itr->Begin(); itr->End() == aFALSE; itr->Next())
                    values.insert(values.begin() + lkey, itr->Valuedecimal());
                itr->Release();
                return aTRUE;
            }

            values.insert(values.begin() + lkey, ke->Decimal());
        }
        else {
            long ikey = idx->Integer();
            long sz = values.size();
            long nb;
            float n = ke->Decimal();
            if (ikey < 0) {
                ikey = sz + ikey;
                if (ikey < 0)
                    return globalTamgu->Returnerror("Cannot set this value", idthread);
            }

            if (ikey >= sz) {
                for (nb = values.size(); nb < ikey; nb++)
                    values.push_back(0);
                values.push_back(n);
            }
            else
                values[ikey] = n;
        }
    }
    return aTRUE;
}

Exporting bool Tamgudvector::Permute() {
    Locking _lock(this);

    return next_permutation(values.begin(), values.end());
}

Exporting void Tamgudvector::Shuffle() {
    Locking _lock(this);

    size_t sz = values.size();
    size_t i, f;
    long mx = sz;
    float v;
    for (i = 0; i < sz; i++) {
        f = localrandom(mx--);
        if (mx != f) {
            v = values[mx];
            values[mx] = values[f];
            values[f] = v;
        }
    }
}

Exporting Tamgu* Tamgudvector::Unique() {
    Locking _lock(this);
    Tamgudvector* kvect = new Tamgudvector;
    hmap<float, bool> inter;
    for (int i = 0; i < values.size(); i++) {
        if (inter.find(values[i]) == inter.end()) {
            inter[values[i]] = true;
            kvect->values.push_back(values[i]);
        }
    }
    return kvect;
}


Exporting Tamgu* Tamgudvector::Eval(Tamgu* contextualpattern, Tamgu* idx, short idthread) {
    Locking _lock(this);

    if (!idx->isIndex()) {
        if (contextualpattern->isLoop())
            return this;

        //particular case, the contextualpattern is a vector, which means that we expect a set of keys
        //as a result
        if (contextualpattern->isVectorContainer()) {
            Tamgu* kv = Selectadvector(contextualpattern);
            if (kv->Type() == a_dvector)
                ((Tamgudvector*)kv)->values = values;
            else
                for (int i = 0; i < values.size(); i++)
                    kv->Push(new Tamgudecimal(values[i]));
            return kv;
        }

        if (contextualpattern->isNumber()) {
            long v = Size();
            return globalTamgu->Provideint(v);
        }

        return this;
    }

    long ikey;
    Tamgu* keyright = NULL;
    ikey = idx->Getinteger(idthread);
    if (idx->isInterval())
        keyright = ((TamguIndex*)idx)->right->Eval(aNULL, aNULL, idthread);

    if (ikey < 0)
        ikey = values.size() + ikey;

    if (ikey < 0 || ikey >= values.size()) {
        if (ikey != values.size() || keyright == NULL) {
            if (globalTamgu->erroronkey)
                return globalTamgu->Returnerror("Wrong index", idthread);
            return aNOELEMENT;
        }
    }

    if (keyright == NULL)
        return new Tamgudecimal(values[ikey]);

    Tamgudvector* kvect;

    long iright = keyright->Integer();
    keyright->Release();
    if (iright < 0 || keyright == aNULL) {
        iright = values.size() + iright;
        if (iright<ikey) {
            if (globalTamgu->erroronkey)
                return globalTamgu->Returnerror("Wrong index", idthread);
            return aNOELEMENT;
        }
    }
    else {
        if (iright>values.size()) {
            if (globalTamgu->erroronkey)
                return globalTamgu->Returnerror("Wrong index", idthread);
            return aNOELEMENT;
        }
    }
    //In this case, we must create a new vector
    kvect = new Tamgudvector;
    for (long i = ikey; i < iright; i++)
        kvect->values.push_back(values[i]);
    return kvect;
}

Exporting Tamgu* Tamgudvector::Map(short idthread) {
    Locking _lock(this);
    Tamgumap* kmap = globalTamgu->Providemap();

    char buff[100];
    for (int it = 0; it < values.size(); it++) {
        sprintf_s(buff, 100, "%d", it);
        kmap->Push(buff, new Tamgudecimal(values[it]));
    }
    return kmap;
}

Exporting Tamgu* Tamgudvector::Vector(short idthread) {
    Locking _lock(this);
    Tamguvector* kvect = globalTamgu->Providevector();
    kvect->values.reserve(values.size());
    for (int i = 0; i < values.size(); i++)
        kvect->Push(new Tamgudecimal(values[i]));
    return kvect;
}



Exporting Tamgu* Tamgudvector::same(Tamgu* a) {
    if (!a->isVectorContainer())
        return aFALSE;


    Doublelocking _lock(this, a);

    if (a->Type() != idtype) {
        if (a->isVectorContainer()) {
            if (a->Size() != values.size())
                return aFALSE;
            for (int i = 0; i < values.size(); i++) {
                if (values[i] != a->getdecimal(i))
                    return aFALSE;
            }
            return aTRUE;
        }
        return aFALSE;
    }

    Tamgudvector* m = (Tamgudvector*)a;
    if (m->values.size() != values.size())
        return aFALSE;

    for (int i = 0; i < values.size(); i++) {
        if (m->values[i] != values[i])
            return aFALSE;
    }
    return aTRUE;
}


Exporting Tamgu* Tamgudvector::andset(Tamgu* b, bool itself) {
    Tamgudvector* ref = this;
    if (itself)
        ref = this;
    else
        ref = (Tamgudvector*)Atom(true);

    Doublelocking _lock(ref, b);
    size_t it;
    long v;
    if (b->isContainer()) {
        TamguIteration* itr = b->Newiteration(false);
        itr->Begin();
        for (it = 0; it < ref->values.size(); it++) {
            if (itr->End() == aTRUE)
                break;
            v = ref->values[it];
            v &= itr->Valueinteger();
            ref->values[it] = v;
            itr->Next();
        }
        itr->Release();
        return ref;
    }

    v = b->Integer();
    long vbis;
    for (it = 0; it < ref->values.size(); it++) {
        vbis = ref->values[it];
        vbis &= v;
        ref->values[it] = vbis;
    }
    return ref;
}

Exporting Tamgu* Tamgudvector::orset(Tamgu* b, bool itself) {
    Tamgudvector* ref = this;
    if (itself)
        ref = this;
    else
        ref = (Tamgudvector*)Atom(true);

    Doublelocking _lock(ref, b);
    size_t it;
    long v;
    if (b->isContainer()) {
        TamguIteration* itr = b->Newiteration(false);
        itr->Begin();
        for (it = 0; it < ref->values.size(); it++) {
            if (itr->End() == aTRUE)
                break;
            v = ref->values[it];
            v |= itr->Valueinteger();
            ref->values[it] = v;
            itr->Next();
        }
        itr->Release();
        return ref;
    }

    v = b->Integer();
    long vbis;
    for (it = 0; it < ref->values.size(); it++) {
        vbis = ref->values[it];
        vbis |= v;
        ref->values[it] = vbis;
    }
    return ref;
}

Exporting Tamgu* Tamgudvector::xorset(Tamgu* b, bool itself) {
    Tamgudvector* ref = this;
    if (itself)
        ref = this;
    else
        ref = (Tamgudvector*)Atom(true);

    Doublelocking _lock(ref, b);
    size_t it;
    long v;
    if (b->isVectorContainer()) {
        long size = Size();

        Tamgudvector* ref = new Tamgudvector;
        Locking _lock(this);
        hmap<float, bool> store;
        for (it = 0; it < b->Size(); it++)
            store[b->getdecimal(it)] = true;

        for (it = 0; it < size; it++) {
            if (store.find(values[it]) == store.end())
                store[values[it]] = true;
        }

        for (auto& i : store)
            ref->values.push_back(i.first);

        return ref;
    }

    v = b->Integer();
    long vbis;
    for (it = 0; it < ref->values.size(); it++) {
        vbis = ref->values[it];
        vbis ^= v;
        ref->values[it] = vbis;
    }
    return ref;
}


Exporting Tamgu* Tamgudvector::plus(Tamgu* b, bool itself) {
    Tamgudvector* ref = this;
    if (itself)
        ref = this;
    else
        ref = (Tamgudvector*)Atom(true);

    Doublelocking _lock(ref, b);
    size_t it;

    if (b->isContainer()) {
        TamguIteration* itr = b->Newiteration(false);
        itr->Begin();
        for (it = 0; it < ref->values.size(); it++) {
            if (itr->End() == aTRUE)
                break;
            ref->values[it] += itr->Valuedecimal();
            itr->Next();
        }
        itr->Release();
        return ref;
    }

    float v = b->Decimal();
    for (it = 0; it < ref->values.size(); it++)
        ref->values[it] += v;
    return ref;
}

Exporting Tamgu* Tamgudvector::minus(Tamgu* b, bool itself) {
    Tamgudvector* ref = this;
    if (itself)
        ref = this;
    else
        ref = (Tamgudvector*)Atom(true);

    Doublelocking _lock(ref, b);
    size_t it;

    if (b->isContainer()) {
        TamguIteration* itr = b->Newiteration(false);
        itr->Begin();
        for (it = 0; it < ref->values.size(); it++) {
            if (itr->End() == aTRUE)
                break;
            ref->values[it] -= itr->Valuedecimal();
            itr->Next();
        }
        itr->Release();
        return ref;
    }

    float v = b->Decimal();
    for (it = 0; it < ref->values.size(); it++)
        ref->values[it] -= v;
    return ref;
}

Exporting Tamgu* Tamgudvector::multiply(Tamgu* b, bool itself) {
    Tamgudvector* ref = this;
    if (itself)
        ref = this;
    else
        ref = (Tamgudvector*)Atom(true);

    Doublelocking _lock(ref, b);
    size_t it;

    if (b->isContainer()) {
        TamguIteration* itr = b->Newiteration(false);
        itr->Begin();
        for (it = 0; it < ref->values.size(); it++) {
            if (itr->End() == aTRUE)
                break;
            ref->values[it] *= itr->Valuedecimal();
            itr->Next();
        }
        itr->Release();
        return ref;
    }

    float v = b->Decimal();
    for (it = 0; it < ref->values.size(); it++)
        ref->values[it] *= v;
    return ref;
}

Exporting Tamgu* Tamgudvector::divide(Tamgu* b, bool itself) {
    Tamgudvector* ref = this;
    if (itself)
        ref = this;
    else
        ref = (Tamgudvector*)Atom(true);

    Doublelocking _lock(ref, b);
    size_t it;
    float v;
    if (b->isContainer()) {
        TamguIteration* itr = b->Newiteration(false);
        itr->Begin();
        for (it = 0; it < ref->values.size(); it++) {
            if (itr->End() == aTRUE)
                break;
            v = itr->Valuedecimal();
            if (v == 0) {
                ref->Release();
                return globalTamgu->Returnerror("Error: Divided by 0");
            }
            ref->values[it] /= v;
            itr->Next();
        }
        itr->Release();
        return ref;
    }

    v = b->Decimal();
    if (v == 0) {
        ref->Release();
        return globalTamgu->Returnerror("Error: Divided by 0");
    }

    for (it = 0; it < ref->values.size(); it++)
        ref->values[it] /= v;
    return ref;
}

Exporting Tamgu* Tamgudvector::power(Tamgu* b, bool itself) {
    Tamgudvector* ref = this;
    if (itself)
        ref = this;
    else
        ref = (Tamgudvector*)Atom(true);

    Doublelocking _lock(ref, b);
    size_t it;

    if (b->isContainer()) {
        TamguIteration* itr = b->Newiteration(false);
        itr->Begin();
        for (it = 0; it < ref->values.size(); it++) {
            if (itr->End() == aTRUE)
                break;
            ref->values[it] = pow(ref->values[it], itr->Valuedecimal());
            itr->Next();
        }
        itr->Release();
        return ref;
    }

    float v = b->Decimal();
    for (it = 0; it < ref->values.size(); it++)
        ref->values[it] = pow(ref->values[it], v);

    return ref;
}

Exporting Tamgu* Tamgudvector::shiftleft(Tamgu* b, bool itself) {
    Tamgudvector* ref = this;
    if (itself)
        ref = this;
    else
        ref = (Tamgudvector*)Atom(true);

    Doublelocking _lock(ref, b);
    size_t it;
    long v;
    if (b->isContainer()) {
        TamguIteration* itr = b->Newiteration(false);
        itr->Begin();
        for (it = 0; it < ref->values.size(); it++) {
            if (itr->End() == aTRUE)
                break;
            v = ref->values[it];
            v <<= itr->Valueinteger();
            ref->values[it] = v;
            itr->Next();
        }
        itr->Release();
        return ref;
    }

    v = b->Integer();
    long vint;
    for (it = 0; it < ref->values.size(); it++) {
        vint = ref->values[it];
        vint <<= v;
        ref->values[it] = vint;
    }
    return ref;
}

Exporting Tamgu* Tamgudvector::shiftright(Tamgu* b, bool itself) {
    Tamgudvector* ref = this;
    if (itself)
        ref = this;
    else
        ref = (Tamgudvector*)Atom(true);

    Doublelocking _lock(ref, b);
    size_t it;
    long v;
    if (b->isContainer()) {
        TamguIteration* itr = b->Newiteration(false);
        itr->Begin();
        for (it = 0; it < ref->values.size(); it++) {
            if (itr->End() == aTRUE)
                break;
            v = ref->values[it];
            v >>= itr->Valueinteger();
            ref->values[it] = v;
            itr->Next();
        }
        itr->Release();
        return ref;
    }

    v = b->Integer();
    long vint;
    for (it = 0; it < ref->values.size(); it++) {
        vint = ref->values[it];
        vint >>= v;
        ref->values[it] = vint;
    }
    return ref;
}

Exporting Tamgu* Tamgudvector::mod(Tamgu* b, bool itself) {
    Tamgudvector* ref = this;
    if (itself)
        ref = this;
    else
        ref = (Tamgudvector*)Atom(true);

    Doublelocking _lock(ref, b);
    size_t it;
    long v;
    long vbis;
    if (b->isContainer()) {
        TamguIteration* itr = b->Newiteration(false);
        itr->Begin();
        for (it = 0; it < ref->values.size(); it++) {
            if (itr->End() == aTRUE)
                break;
            v = itr->Valueinteger();
            if (v == 0) {
                ref->Release();
                return globalTamgu->Returnerror("Error: Divided by 0");
            }
            vbis = ref->values[it];
            vbis %= v;
            ref->values[it] = vbis;
            itr->Next();
        }
        itr->Release();
        return ref;
    }

    v = b->Integer();
    if (v == 0) {
        ref->Release();
        return globalTamgu->Returnerror("Error: Divided by 0");
    }

    for (it = 0; it < ref->values.size(); it++) {
        vbis = ref->values[it];
        vbis %= v;
        ref->values[it] = vbis;

    }
    return ref;
}

Exporting Tamgu* Tamgudvector::Looptaskell(Tamgu* recipient, Tamgu* context, Tamgu* environment, TamguFunctionLambda* bd, short idthread) {
    Tamgu* a;
    Tamgudecimal* it = new Tamgudecimal(0);
    it->Setreference();
    
    uchar addvalue = 0;

    if (context != aNOELEMENT)
        addvalue = Selecttype(context);

    recipient->Putvalue( it, idthread);

    for (size_t i = 0; i < values.size(); i++) {
        it->value = values[i];

        a = bd->DirectEval(environment, aNULL, idthread);
        if (a->isNULL())
            continue;

        if (globalTamgu->Error(idthread) || a->needInvestigate()) {
            if (a == aBREAK)
                break;

            it->Resetreference();
            recipient->Forcedclean();
            a->Release();
            context->Release();
            return globalTamgu->Errorobject(idthread);
        }

        context = Storealongtype(context, a, idthread, addvalue);
        a->Release();
    }

    it->Resetreference();
    recipient->Forcedclean();
    return context;
}

Exporting Tamgu* Tamgudvector::Filter(short idthread, Tamgu* env, TamguFunctionLambda* bd, Tamgu* var, Tamgu* kcont, Tamgu* accu, Tamgu* init, bool direct) {

    Tamgu* returnval;

    bool first = false;
    
    Tamgudecimal* key = new Tamgudecimal(0);
    key->Setreference();

    if (init != aNOELEMENT) {
        accu->Putvalue(init, idthread);
        if (kcont != NULL) {
            if (direct)
                kcont->Insert(0, init);
            else
                kcont->Push(init);
        }
    }
    else
        first = true; //we are dealing with a foldr1 or a foldl1
        
    for (size_t i = 0; i < values.size(); i++) {
        key->value = values[i];
        if (first) {
            returnval = key->Atom();//We use the first value of the list to seed our accumulator variable
            first = false;
        }
        else {
            var->Putvalue(key, idthread);
            returnval = bd->DirectEval(env, aNULL, idthread);

            if (returnval == aBREAK) {
                accu = returnval;
                break;
            }

            if (globalTamgu->Error(idthread)) {
                key->Resetreference();
                var->Forcedclean();
                if (kcont != NULL)
                    kcont->Release();
                return globalTamgu->Errorobject(idthread);
            }
        }

        if (returnval != aNULL) {
            
            accu->Putvalue(returnval, idthread);
            if (kcont != NULL) {
                if (direct)
                    kcont->Insert(0, returnval);
                else
                    kcont->Push(returnval);
            }
            returnval->Release();
        }
    }

    key->Resetreference();
    var->Forcedclean();
    if (kcont != NULL)
        return kcont;
    return accu->Value();
}

class DComp {
    public:
    TamguCallFunction compare;
    short idthread;
    TamguConstDecimal p;
    TamguConstDecimal s;

    DComp(TamguFunction* comp, short idt) : compare(comp), p(0), s(0), idthread(idt) {
        compare.arguments.push_back(&p);
        compare.arguments.push_back(&s);
    }

    bool get() {
        return compare.Eval(aNULL, aNULL, idthread)->Boolean();
    }
};

class DComparison {
    public:
    DComp* compare;

    DComparison(DComp* c)  {
        compare = c;
    }

    bool operator() (float i, float j) {
        compare->p.value = i;
        compare->s.value = j;
        return compare->get();
    }
};

Tamgu* Tamgudvector::MethodSort(Tamgu* contextualpattern, short idthread, TamguCall* callfunc) {
    Tamgu* comp = callfunc->Evaluate(0, contextualpattern, idthread);
    if (comp->isFunction()) {
        if (comp->Size() != 2)
            return globalTamgu->Returnerror("Expecting a comparison function with two parameters", idthread);

        DComp kcomp((TamguFunction*)comp->Body(idthread), idthread);
        DComparison kfcomp(&kcomp);
        sort(values.begin(), values.end(), kfcomp);
        return this;
    }
    Sort(comp->Boolean());
    return this;
}


Exporting Tamgu* Tamgudvector::Loopin(TamguInstruction* ins, Tamgu* context, short idthread) {
    Locking _lock(this);
    Tamgu* var = ins->instructions.vecteur[0]->Instruction(0);
    var = var->Eval(context, aNULL, idthread);

    
    long sz = Size();
    Tamgu* a = aNULL;
    bool testcond = false;
    for (long i = 0; i < sz && !testcond; i++) {
        a->Releasenonconst();
        var->storevalue(values[i]);
        a = ins->instructions.vecteur[1]->Eval(context, aNULL, idthread);

        //Continue does not trigger needInvestigate
        testcond = a->needInvestigate();
    }
    
    if (testcond) {
        if (a == aBREAK)
            return this;
        return a;
    }

    a->Releasenonconst();
    return this;
}
