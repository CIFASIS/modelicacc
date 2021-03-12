S_p: {{[1:1:1]}}
R_p: {{[2:1:1001]}}
S_n: {{[1002:1:1002]}}
G_p: {{[1003:1:1003]}}
R_n: {{[1004:1:2003]}}
C_p: {{[2004:1:3003]}}
C_n: {{[3004:1:4003]}}
E0: [({{[1:1:1]}}, [0 * x + 1])], [({{[1:1:1]}}, [0 * x + 2])]
E1: [({{[2:1:2]}}, [0 * x + 1002])], [({{[2:1:2]}}, [0 * x + 1003])]
E2: [({{[3:1:1001]}}, [1 * x + 1001])], [({{[3:1:1001]}}, [1 * x + 0])]
E3: [({{[1002:1:2001]}}, [1 * x + 1002])], [({{[1002:1:2001]}}, [1 * x + 2])]
E4: [({{[2002:1:3001]}}, [1 * x + 1002])], [({{[2002:1:3001]}}, [0 * x + 1003])]
Generated Connect Graph written to prueba.dot
[({{[2003:1:2003]}}, [1 * x + 0]) , ({{[3003:1:3003]}}, [1 * x -1000]), ({{[3:1:1001]}}, [1 * x + 0]), ({{[1004:1:2002]}}, [1 * x -1001]), ({{[2004:1:3002]}}, [1 * x -2001]), ({{[3004:1:4003]}}, [0 * x + 1002]), ({{[1003:1:1003]}}, [0 * x + 1002]), ({{[1002:1:1002]}}, [1 * x + 0]), ({{[2:1:2]}}, [0 * x + 1]), ({{[1:1:1]}}, [1 * x + 0])]

model Test
  Real N=1000;
  Real S_n_eff;
  flow Real S_n_fl;
  Real S_p_eff;
  flow Real S_p_fl;
  Real R_n_eff[N];
  flow Real R_n_fl[N];
  Real R_p_eff[N];
  flow Real R_p_fl[N];
  Real G_n_eff;
  flow Real G_n_fl;
  Real G_p_eff;
  flow Real G_p_fl;
  Real C_n_eff[N];
  flow Real C_n_fl[N];
  Real C_p_eff[N];
  flow Real C_p_fl[N];
equation
  for i in 1:1:1 loop
    G_p_eff = S_n_eff;
  end for;
  for i in 1:1:1000 loop
    C_n_eff[i] = S_n_eff;
  end for;
  for i in 1:1:1 loop
    S_n_fl+sum(C_n_fl[1:1:1000])+G_p_fl = 0;
  end for;
  for i in 1000:1:1000 loop
    C_p_eff[i] = R_n_eff[1*i+0];
  end for;
  for i in 1000:1:1000 loop
    C_p_fl[1*i+0]+R_n_fl[1*i+0] = 0;
  end for;
  for i in 1:1:999 loop
    C_p_eff[i] = R_p_eff[1*i+1];
  end for;
  for i in 1:1:999 loop
    R_n_eff[i] = R_p_eff[1*i+1];
  end for;
  for i in 2:1:1000 loop
    R_n_fl[1*i+-1]+R_p_fl[1*i+0]+C_p_fl[1*i+-1] = 0;
  end for;
  for i in 1:1:1 loop
    R_p_eff[i] = S_p_eff;
  end for;
  for i in 1:1:1 loop
    S_p_fl+R_p_fl[1*i+0] = 0;
  end for;
end Test;
