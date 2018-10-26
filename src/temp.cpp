//DES* G0 = new xorDES("G_0",nullptr,3,0);
//DES* G00 = new simpleDES("G00",G0,0);
//DES* G01 = new simpleDES("G01",G0,1);

//G0->addComponent(G00);
//G0->addComponent(G01);

//DES* G02 = new syncDES("G_02",G0,2);
//DES* G020 = new xorDES("G020",G02,2,0);
//DES* G0200 = new simpleDES("G0200",G020,0);
//DES* G0201 = new simpleDES("G0201",G020,1);
//G020->addComponent(G0200);
//G020->addComponent(G0201);


//DES* G021 = new xorDES("G021",G02,2,1);
//DES* G0210 = new simpleDES("G0210",G021,0);
//DES* G0211 = new simpleDES("G0211",G021,1);
//G021->addComponent(G0210);
//G021->addComponent(G0211);

//G02->addComponent(G020);
//G02->addComponent(G021);
//G0->addComponent(G02);


//fdd_printset(G0->theta(G0210)); cout << endl;

//event* alpha = new event("alpha",1);
//event* beta  = new event("beta",0);

//G0->addEvent(alpha);
//G0->addEvent(beta);


//transition* t1 = new transition(alpha);
//transition* t2 = new transition(beta);

//try{
//    t1->addSrc(G0201);
//    t1->addSrc(G0211);
//    t1->addDst(G00);

//    t2->addSrc(G01);
//    t2->addDst(G00);
//} catch (badSrcDES ex){
//    cerr << "illegal source added" << endl;
//    abort();
//} catch (badDstDES ex){
//    cerr << "illegal destination added" << endl;
//    abort();
//}

//delta* trS = new delta();
//trS->addTransition(alpha, t1);
//trS->addTransition(beta,  t2);
//G0->addDelta(trS);

//trS->symbolicEncoding(G0);
//fdd_printset(beta->getNSigma()); cout << endl;

//cout << *G0 << endl;
