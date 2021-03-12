R_n: {{[1:1:200000]}}
C1_n: {{[200001:1:300000]}}
R_p: {{[300001:1:500000]}}
C1_p: {{[500001:1:600000]}}
C2_n: {{[600001:1:700000]}}
C2_p: {{[700001:1:800000]}}
E0: [({{[1:1:100000]}}, [2 * x + 0])], [({{[1:1:100000]}}, [1 * x + 200000])]
E1: [({{[100001:1:200000]}}, [2 * x + 100000])], [({{[100001:1:200000]}}, [1 * x + 400000])]
E2: [({{[200001:1:300000]}}, [2 * x -400001])], [({{[200001:1:300000]}}, [1 * x + 400000])]
E3: [({{[300001:1:400000]}}, [2 * x -300001])], [({{[300001:1:400000]}}, [1 * x + 400000])]
Generated Connect Graph written to prueba.dot
[({{[700001:1:800000]}}, [2 * x -1.1e+06]) , ({{[600001:1:700000]}}, [2 * x -1.2e+06]), ({{[500001:1:600000]}}, [2 * x -700000]), ({{[200001:1:300000]}}, [2 * x -400000]), ({{[300001:2:499999]}}, [1 * x + 0]), ({{[1:2:199999]}}, [1 * x + 0]), ({{[300002:2:500000]}}, [1 * x + 0]), ({{[2:2:200000]}}, [1 * x + 0])]

model Test
  Real N=100000;
  Real R_n_eff[2*N];
  flow Real R_n_fl[2*N];
  Real R_p_eff[2*N];
  flow Real R_p_fl[2*N];
  Real C1_n_eff[N];
  flow Real C1_n_fl[N];
  Real C1_p_eff[N];
  flow Real C1_p_fl[N];
  Real C2_n_eff[N];
  flow Real C2_n_fl[N];
  Real C2_p_eff[N];
  flow Real C2_p_fl[N];
equation
  for i in 1:1:100000 loop
    C1_n_eff[i] = R_n_eff[2*i+0];
  end for;
  for i in 2:2:200000 loop
    R_n_fl[1*i+0]+C1_n_fl[0.5*i+0] = 0;
  end for;
  for i in 1:1:100000 loop
    C2_n_eff[i] = R_n_eff[2*i+-1];
  end for;
  for i in 1:2:199999 loop
    R_n_fl[1*i+0]+C2_n_fl[0.5*i+0.5] = 0;
  end for;
  for i in 1:1:100000 loop
    C2_p_eff[i] = R_p_eff[2*i+-1];
  end for;
  for i in 1:2:199999 loop
    C2_p_fl[0.5*i+0.5]+R_p_fl[1*i+0] = 0;
  end for;
  for i in 1:1:100000 loop
    C1_p_eff[i] = R_p_eff[2*i+0];
  end for;
  for i in 2:2:200000 loop
    C1_p_fl[0.5*i+0]+R_p_fl[1*i+0] = 0;
  end for;
end Test;
