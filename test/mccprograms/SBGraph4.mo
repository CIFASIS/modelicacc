model Test
  connector Electrical
    Real eff;
    flow Real fl;
  end Electrical;
  
  model Obj
    Electrical n, p;
  end Obj;

  Real N = 1000;
  Obj R[2*N];
  Obj C1[N];
  Obj C2[N];
equation
  for i in 1:N loop
    connect(R[2 * i].n, C1[i].n);
    connect(R[2 * i].p, C1[i].p);
  end for;

  for i in 1:N loop
    connect(R[2 * i - 1].n, C2[i].n);
    connect(R[2 * i - 1].p, C2[i].p);
  end for;
end Test;
