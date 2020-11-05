model Test
  connector Electrical
    Real eff;
    flow Real fl;
  end Electrical;
  
  model Obj
    Electrical n, p;
  end Obj;

  Real N = 1000;
  Obj S;
  Obj R[N];
  Obj G;
  Obj C[N];
equation
  connect(S.p, R[1].p);
  connect(S.n, G.p);

  for i in 1:N-1 loop
    connect(R[i].n, R[i + 1].p);
  end for;

  for i in 1:N loop
    connect(C[i].p, R[i].n);
    connect(C[i].n, G.p);
  end for;

  for i in 1:N-1 loop
    connect(C[i].n, C[i + 1].n);
  end for;
end Test;
