model N2D_dassl_10
  constant Integer N=10;
  constant Integer M=10;
  parameter Real Cell_resistor_R[N, M](each start=1);
  parameter Real Cell_resistor_T_ref[N, M]=fill(300.15,N,M);
  parameter Real Cell_resistor_alpha[N, M]=fill(0,N,M);
  Real Cell_resistor_v[N, M];
  Real Cell_resistor_i[N, M];
  Real Cell_resistor_p_v[N, M];
  Real Cell_resistor_p_i[N, M];
  Real Cell_resistor_n_v[N, M];
  Real Cell_resistor_n_i[N, M];
  Real Cell_resistor_LossPower[N, M];
  Real Cell_resistor_T_heatPort[N, M](each start=288.15);
  Real Cell_resistor_R_actual[N, M];
  parameter Real Cell_resistor1_R[N, M](each start=1);
  parameter Real Cell_resistor1_T_ref[N, M]=fill(300.15,N,M);
  parameter Real Cell_resistor1_alpha[N, M]=fill(0,N,M);
  Real Cell_resistor1_v[N, M];
  Real Cell_resistor1_i[N, M];
  Real Cell_resistor1_p_v[N, M];
  Real Cell_resistor1_p_i[N, M];
  Real Cell_resistor1_n_v[N, M];
  Real Cell_resistor1_n_i[N, M];
  Real Cell_resistor1_LossPower[N, M];
  Real Cell_resistor1_T_heatPort[N, M](each start=288.15);
  Real Cell_resistor1_R_actual[N, M];
  Real Cell_capacitor_v[N, M](each start=0);
  Real Cell_capacitor_i[N, M];
  Real Cell_capacitor_p_v[N, M];
  Real Cell_capacitor_p_i[N, M];
  Real Cell_capacitor_n_v[N, M];
  Real Cell_capacitor_n_i[N, M];
  parameter Real Cell_capacitor_C[N, M](each start=1);
  Real Cell_capacitor1_v[N, M](each start=0);
  Real Cell_capacitor1_i[N, M];
  Real Cell_capacitor1_p_v[N, M];
  Real Cell_capacitor1_p_i[N, M];
  Real Cell_capacitor1_n_v[N, M];
  Real Cell_capacitor1_n_i[N, M];
  parameter Real Cell_capacitor1_C[N, M](each start=1);
  Real Cell_l_v[N, M];
  Real Cell_l_i[N, M];
  Real Cell_d_v[N, M];
  Real Cell_d_i[N, M];
  Real Cell_r_v[N, M];
  Real Cell_r_i[N, M];
  Real Cell_u_v[N, M];
  Real Cell_u_i[N, M];
  parameter Real S_V(start=1);
  Real S_v;
  Real S_i;
  Real S_p_v;
  Real S_p_i;
  Real S_n_v;
  Real S_n_i;
  Real G_p_v;
  Real G_p_i;
equation
  for _Index_0 in 1:N,_Index_1 in 1:M loop
    Cell_resistor_R_actual[_Index_0,_Index_1] = Cell_resistor_R[_Index_0,_Index_1]*(1+Cell_resistor_alpha[_Index_0,_Index_1]*(Cell_resistor_T_heatPort[_Index_0,_Index_1]-Cell_resistor_T_ref[_Index_0,_Index_1]));
    Cell_resistor_v[_Index_0,_Index_1] = Cell_resistor_R_actual[_Index_0,_Index_1]*Cell_resistor_i[_Index_0,_Index_1];
    Cell_resistor_LossPower[_Index_0,_Index_1] = Cell_resistor_v[_Index_0,_Index_1]*Cell_resistor_i[_Index_0,_Index_1];
    Cell_resistor_T_heatPort[_Index_0,_Index_1] = Cell_resistor_T_ref[_Index_0,_Index_1];
    Cell_resistor_v[_Index_0,_Index_1] = Cell_resistor_p_v[_Index_0,_Index_1]-Cell_resistor_n_v[_Index_0,_Index_1];
    0 = Cell_resistor_p_i[_Index_0,_Index_1]+Cell_resistor_n_i[_Index_0,_Index_1];
    Cell_resistor_i[_Index_0,_Index_1] = Cell_resistor_p_i[_Index_0,_Index_1];
    Cell_resistor1_R_actual[_Index_0,_Index_1] = Cell_resistor1_R[_Index_0,_Index_1]*(1+Cell_resistor1_alpha[_Index_0,_Index_1]*(Cell_resistor1_T_heatPort[_Index_0,_Index_1]-Cell_resistor1_T_ref[_Index_0,_Index_1]));
    Cell_resistor1_v[_Index_0,_Index_1] = Cell_resistor1_R_actual[_Index_0,_Index_1]*Cell_resistor1_i[_Index_0,_Index_1];
    Cell_resistor1_LossPower[_Index_0,_Index_1] = Cell_resistor1_v[_Index_0,_Index_1]*Cell_resistor1_i[_Index_0,_Index_1];
    Cell_resistor1_T_heatPort[_Index_0,_Index_1] = Cell_resistor1_T_ref[_Index_0,_Index_1];
    Cell_resistor1_v[_Index_0,_Index_1] = Cell_resistor1_p_v[_Index_0,_Index_1]-Cell_resistor1_n_v[_Index_0,_Index_1];
    0 = Cell_resistor1_p_i[_Index_0,_Index_1]+Cell_resistor1_n_i[_Index_0,_Index_1];
    Cell_resistor1_i[_Index_0,_Index_1] = Cell_resistor1_p_i[_Index_0,_Index_1];
    Cell_capacitor_i[_Index_0,_Index_1] = Cell_capacitor_C[_Index_0,_Index_1]*der(Cell_capacitor_v[_Index_0,_Index_1]);
    Cell_capacitor_v[_Index_0,_Index_1] = Cell_capacitor_p_v[_Index_0,_Index_1]-Cell_capacitor_n_v[_Index_0,_Index_1];
    0 = Cell_capacitor_p_i[_Index_0,_Index_1]+Cell_capacitor_n_i[_Index_0,_Index_1];
    Cell_capacitor_i[_Index_0,_Index_1] = Cell_capacitor_p_i[_Index_0,_Index_1];
    Cell_capacitor1_i[_Index_0,_Index_1] = Cell_capacitor1_C[_Index_0,_Index_1]*der(Cell_capacitor1_v[_Index_0,_Index_1]);
    Cell_capacitor1_v[_Index_0,_Index_1] = Cell_capacitor1_p_v[_Index_0,_Index_1]-Cell_capacitor1_n_v[_Index_0,_Index_1];
    0 = Cell_capacitor1_p_i[_Index_0,_Index_1]+Cell_capacitor1_n_i[_Index_0,_Index_1];
    Cell_capacitor1_i[_Index_0,_Index_1] = Cell_capacitor1_p_i[_Index_0,_Index_1];
  end for;
  S_v = S_V;
  S_v = S_p_v-S_n_v;
  0 = S_p_i+S_n_i;
  S_i = S_p_i;
  G_p_v = 0;
  for i in 1:1:9,j in 1:1:9 loop
    Cell_d_i[i,j]+Cell_u_i[i+1,j] = 0;
  end for;
  Cell_u_v[1,1] = S_p_v;
  for i in 1:1:1,j in 1:1:9 loop
    Cell_u_v[1,1] = Cell_u_v[1,j+1];
  end for;
  Cell_u_i[1,1]+S_p_i+sum(Cell_u_i[1:1:1, 2:1:10]) = 0;
  Cell_d_v[10,1] = S_n_v;
  for i in 1:1:1,j in 1:1:9 loop
    Cell_d_v[10,1] = Cell_d_v[10,j+1];
  end for;
  Cell_d_i[10,1]+S_n_i+sum(Cell_d_i[10:1:10, 2:1:10]) = 0;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_resistor_p_i[i,j]+(-Cell_l_i[i,j]) = 0;
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_capacitor_n_i[i,j]+(-Cell_r_i[i,j]) = 0;
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_resistor1_p_i[i,j]+(-Cell_u_i[i,j]) = 0;
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_capacitor1_n_i[i,j]+(-Cell_d_i[i,j]) = 0;
  end for;
  for i in 1:1:9,j in 1:1:9 loop
    Cell_l_i[i,j+1]+Cell_r_i[i,j] = 0;
  end for;
  for i in 1:1:10,j in 1:1:1 loop
    Cell_l_v[i,1] = Cell_r_v[i,10];
  end for;
  for i in 1:1:10,j in 1:1:1 loop
    Cell_l_i[i,1]+Cell_r_i[i,10] = 0;
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_capacitor1_p_i[i,j]+Cell_resistor1_n_i[i,j]+Cell_capacitor_p_i[i,j]+Cell_resistor_n_i[i,j] = 0;
  end for;
  for i in 1:1:9,j in 1:1:1 loop
    Cell_u_i[i+1,10] = 0;
  end for;
  for i in 1:1:9,j in 1:1:1 loop
    Cell_d_i[i,10] = 0;
  end for;
  for i in 1:1:1,j in 1:1:9 loop
    Cell_l_i[10,j+1] = 0;
  end for;
  for i in 1:1:1,j in 1:1:9 loop
    Cell_r_i[10,j] = 0;
  end for;
  G_p_i = 0;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_capacitor1_p_v[i,j] = Cell_resistor1_n_v[i,j];
    Cell_capacitor1_p_v[i,j] = Cell_capacitor_p_v[i,j];
    Cell_capacitor1_p_v[i,j] = Cell_resistor_n_v[i,j];
  end for;
  for i in 1:1:9,j in 1:1:9 loop
    Cell_l_v[i,j+1] = Cell_r_v[i,j];
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_capacitor1_n_v[i,j] = Cell_d_v[i,j];
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_resistor1_p_v[i,j] = Cell_u_v[i,j];
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_capacitor_n_v[i,j] = Cell_r_v[i,j];
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_resistor_p_v[i,j] = Cell_l_v[i,j];
  end for;
  for i in 1:1:9,j in 1:1:9 loop
    Cell_d_v[i,j] = Cell_u_v[i+1,j];
  end for;
end N2D_dassl_10;
