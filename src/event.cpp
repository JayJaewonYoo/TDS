#include "event.h"

//constructors
event::event():nSigma{bddtrue}, setOfSrcVars{bddtrue},
    setOfDstVars{bddtrue}, fSigma{bddtrue}{}
event::event(string sigma): name{sigma}, ctrb{true}, nSigma{bddtrue},
    setOfSrcVars{bddtrue}, setOfDstVars{bddtrue}, fSigma{bddtrue}{}
event::event(string sigma, bool _ctrb): name{sigma},
    setOfSrcVars{bddtrue}, setOfDstVars{bddtrue},
    ctrb{_ctrb}, nSigma{bddtrue}, fSigma{bddtrue}{}
//
string event::getName(){return name;}
string event::printName(){
    string ctrb = (this->isControllable())? "(con)":"(unc)";
    return this->getName()+ctrb;
}
bool event::isControllable(){return ctrb;}
void event::addContainer(DES* G){
    container.insert(G);
}

void event::insertSrc(int i){srcVars.insert(i);}
void event::insertDst(int i){dstVars.insert(i);}

void event::setNSigma(bdd P){nSigma &= P;}
bdd  event::getNSigma(){return nSigma;}


void event::setVarSets(){
    psrcToSrc = bdd_newpair();
    srcToPsrc = bdd_newpair();
    vector<int> temp1, temp2, temp3, temp4;
    copy(srcVars.begin(), srcVars.end(), back_inserter(temp1));
    copy(dstVars.begin(), dstVars.end(), back_inserter(temp3));
    for(auto& i:temp1){
        setOfSrcVars &= fdd_ithset(i-1);
        temp2.push_back(i - 1);
    }
    for(auto& i:temp3){
        setOfDstVars &= fdd_ithset(i);
        temp4.push_back(i - 1);
    }
    fdd_setpairs(srcToPsrc,&temp1[0], &temp2[0], temp1.size());
    fdd_setpairs(psrcToSrc,&temp2[0], &temp1[0], temp1.size());
}

bdd event::delta(bdd pred){
    return bdd_exist(
                        bdd_replace(pred,srcToPsrc) & nSigma
                        ,
                        setOfSrcVars);
}

bdd event::gamma(bdd pred){
    return bdd_replace(
                        bdd_exist( pred & nSigma
                        ,
                        setOfDstVars), psrcToSrc);
}

bdd event::getFSigma(){return fSigma;}
void event::setFSigma(bdd P){
    bdd nextG = bdd_exist(nSigma,setOfSrcVars);
    bdd nGood = nextG & P;
    fSigma = this->gamma(nGood);
}

bool eCmp::operator()(const ePtr& e1, const ePtr& e2)const{
    return e1->getName() < e2->getName();
}
