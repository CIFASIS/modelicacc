model Test
  connector Electrical
    Real eff;
    flow Real fl;
  end Electrical;
  
  model Obj
    Electrical n, p;
  end Obj;

  model Cell
    Electrical u, d, l, r;
  end Cell;

  Real N = 1000;
  Real M = 1000;
  Obj S[1, 1];
  Obj G[1, 1];
  Cell C[N, M];
equation
  connect(S.n, G.p);

  for i in 1:N, j in 1:M-1 loop
    connect(C[i, j + 1].l, C[i, j].r);
  end for;

  for j in 1:M loop
    connect(S[1,1].p, C[1, j].u);
    connect(C[N, j].d, G[1,1].p);
  end for;

  for i in 1:N-1, j in 1:M loop
    connect(C[i + 1, j].u, C[i, j].d);
  end for;

  for i in 1:N loop
    connect(C[i, 1].l, C[i, M].r);
  end for;
end Test;
