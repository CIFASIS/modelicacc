model N2D
  constant Integer N=10;
  constant Integer M=10;
  parameter Real Cell_R1_R[N, M](each start=1);
  parameter Real Cell_R1_T_ref[N, M]=fill(300.15,N,M);
  parameter Real Cell_R1_alpha[N, M]=fill(0,N,M);
  Real Cell_R1_v[N, M];
  Real Cell_R1_i[N, M];
  Real Cell_R1_p_v[N, M];
  Real Cell_R1_p_i[N, M];
  Real Cell_R1_n_v[N, M];
  Real Cell_R1_n_i[N, M];
  Real Cell_R1_LossPower[N, M];
  Real Cell_R1_T_heatPort[N, M](each start=288.15);
  Real Cell_R1_R_actual[N, M];
  parameter Real Cell_R2_R[N, M](each start=1);
  parameter Real Cell_R2_T_ref[N, M]=fill(300.15,N,M);
  parameter Real Cell_R2_alpha[N, M]=fill(0,N,M);
  Real Cell_R2_v[N, M];
  Real Cell_R2_i[N, M];
  Real Cell_R2_p_v[N, M];
  Real Cell_R2_p_i[N, M];
  Real Cell_R2_n_v[N, M];
  Real Cell_R2_n_i[N, M];
  Real Cell_R2_LossPower[N, M];
  Real Cell_R2_T_heatPort[N, M](each start=288.15);
  Real Cell_R2_R_actual[N, M];
  Real Cell_C1_v[N, M](each start=0);
  Real Cell_C1_i[N, M];
  Real Cell_C1_p_v[N, M];
  Real Cell_C1_p_i[N, M];
  Real Cell_C1_n_v[N, M];
  Real Cell_C1_n_i[N, M];
  parameter Real Cell_C1_C[N, M](each start=1);
  Real Cell_C2_v[N, M](each start=0);
  Real Cell_C2_i[N, M];
  Real Cell_C2_p_v[N, M];
  Real Cell_C2_p_i[N, M];
  Real Cell_C2_n_v[N, M];
  Real Cell_C2_n_i[N, M];
  parameter Real Cell_C2_C[N, M](each start=1);
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
    Cell_R1_R_actual[_Index_0,_Index_1] = Cell_R1_R[_Index_0,_Index_1]*(1+Cell_R1_alpha[_Index_0,_Index_1]*(Cell_R1_T_heatPort[_Index_0,_Index_1]-Cell_R1_T_ref[_Index_0,_Index_1]));
    Cell_R1_v[_Index_0,_Index_1] = Cell_R1_R_actual[_Index_0,_Index_1]*Cell_R1_i[_Index_0,_Index_1];
    Cell_R1_LossPower[_Index_0,_Index_1] = Cell_R1_v[_Index_0,_Index_1]*Cell_R1_i[_Index_0,_Index_1];
    Cell_R1_T_heatPort[_Index_0,_Index_1] = Cell_R1_T_ref[_Index_0,_Index_1];
    Cell_R1_v[_Index_0,_Index_1] = Cell_R1_p_v[_Index_0,_Index_1]-Cell_R1_n_v[_Index_0,_Index_1];
    0 = Cell_R1_p_i[_Index_0,_Index_1]+Cell_R1_n_i[_Index_0,_Index_1];
    Cell_R1_i[_Index_0,_Index_1] = Cell_R1_p_i[_Index_0,_Index_1];
    Cell_R2_R_actual[_Index_0,_Index_1] = Cell_R2_R[_Index_0,_Index_1]*(1+Cell_R2_alpha[_Index_0,_Index_1]*(Cell_R2_T_heatPort[_Index_0,_Index_1]-Cell_R2_T_ref[_Index_0,_Index_1]));
    Cell_R2_v[_Index_0,_Index_1] = Cell_R2_R_actual[_Index_0,_Index_1]*Cell_R2_i[_Index_0,_Index_1];
    Cell_R2_LossPower[_Index_0,_Index_1] = Cell_R2_v[_Index_0,_Index_1]*Cell_R2_i[_Index_0,_Index_1];
    Cell_R2_T_heatPort[_Index_0,_Index_1] = Cell_R2_T_ref[_Index_0,_Index_1];
    Cell_R2_v[_Index_0,_Index_1] = Cell_R2_p_v[_Index_0,_Index_1]-Cell_R2_n_v[_Index_0,_Index_1];
    0 = Cell_R2_p_i[_Index_0,_Index_1]+Cell_R2_n_i[_Index_0,_Index_1];
    Cell_R2_i[_Index_0,_Index_1] = Cell_R2_p_i[_Index_0,_Index_1];
    Cell_C1_i[_Index_0,_Index_1] = Cell_C1_C[_Index_0,_Index_1]*der(Cell_C1_v[_Index_0,_Index_1]);
    Cell_C1_v[_Index_0,_Index_1] = Cell_C1_p_v[_Index_0,_Index_1]-Cell_C1_n_v[_Index_0,_Index_1];
    0 = Cell_C1_p_i[_Index_0,_Index_1]+Cell_C1_n_i[_Index_0,_Index_1];
    Cell_C1_i[_Index_0,_Index_1] = Cell_C1_p_i[_Index_0,_Index_1];
    Cell_C2_i[_Index_0,_Index_1] = Cell_C2_C[_Index_0,_Index_1]*der(Cell_C2_v[_Index_0,_Index_1]);
    Cell_C2_v[_Index_0,_Index_1] = Cell_C2_p_v[_Index_0,_Index_1]-Cell_C2_n_v[_Index_0,_Index_1];
    0 = Cell_C2_p_i[_Index_0,_Index_1]+Cell_C2_n_i[_Index_0,_Index_1];
    Cell_C2_i[_Index_0,_Index_1] = Cell_C2_p_i[_Index_0,_Index_1];
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
    Cell_R1_p_i[i,j]+(-Cell_l_i[i,j]) = 0;
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_C1_n_i[i,j]+(-Cell_r_i[i,j]) = 0;
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_R2_p_i[i,j]+(-Cell_u_i[i,j]) = 0;
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_C2_n_i[i,j]+(-Cell_d_i[i,j]) = 0;
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
    Cell_C1_p_i[i,j]+Cell_R1_n_i[i,j]+Cell_R2_n_i[i,j]+Cell_C2_p_i[i,j] = 0;
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
    Cell_C1_p_v[i,j] = Cell_R1_n_v[i,j];
    Cell_C1_p_v[i,j] = Cell_R2_n_v[i,j];
    Cell_C1_p_v[i,j] = Cell_C2_p_v[i,j];
  end for;
  for i in 1:1:9,j in 1:1:9 loop
    Cell_l_v[i,j+1] = Cell_r_v[i,j];
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_C2_n_v[i,j] = Cell_d_v[i,j];
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_R2_p_v[i,j] = Cell_u_v[i,j];
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_C1_n_v[i,j] = Cell_r_v[i,j];
  end for;
  for i in 1:1:10,j in 1:1:10 loop
    Cell_R1_p_v[i,j] = Cell_l_v[i,j];
  end for;
  for i in 1:1:9,j in 1:1:9 loop
    Cell_d_v[i,j] = Cell_u_v[i+1,j];
  end for;
end N2D;
