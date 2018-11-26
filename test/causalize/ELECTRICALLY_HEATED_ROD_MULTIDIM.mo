model ELECTRICALLY_HEATED_ROD_MULTIDIM
  constant Integer N = 5;
	constant Integer M = 5;
  parameter Real delR=1, delX=1, p_elec=1, deltav=1, r=1, omega=1, k1=1, k2=1, Troom=1;
	Real T[N,M], T0[M], TL[M], dTdrR[M];
equation
  der(T[1,1]) = omega * ((T[2,1] - 2 * T[1,1] + T0[1]) / delR + (T[2,1]-T0[1]) / (r * 2 * delR) + (T[1,2] - 2 * T[1,1] + Troom) / delX + p_elec / deltav);
  der(T[N,1]) = omega * ((TL[1] - 2 * T[N,1] + T0[1]) / delR + (TL[1]-T[N-1,1]) / (r * 2 * delR) + (T[N,2] - 2 * T[N,1] + Troom) / delX + p_elec / deltav);
  der(T[N,M]) = omega * ((TL[M] - 2 * T[N,M] + T0[M]) / delR + (TL[M]-T[N-1,M]) / (r * 2 * delR) + (Troom - 2 * T[N,M] + T[N,M-1]) / delX + p_elec / deltav);
  der(T[1,M]) = omega * ((T[2,M] - 2 * T[1,M] + T0[M]) / delR + (T[2,M]-T0[M]) / (r * 2 * delR) + (Troom - 2 * T[1,M] + T[1,M-1]) / delX + p_elec / deltav);
  for i in 2:N-1 loop
		der (T[i,1]) = omega * ((T[i+1,1] - 2 * T[i,1] + T[i-1,1]) / delR + (T[i+1,1]-T[i-1,1]) / (r * 2 * delR) + (T[i,2] - 2 * T[i,1] + Troom) / delX + p_elec / deltav);
    der (T[i,M]) = omega * ((T[i+1,M] - 2 * T[i,M] + T[i-1,M]) / delR + (T[i+1,M]-T[i-1,M]) / (r * 2 * delR) + (Troom - 2 * T[i,M] + T[i,M-1]) / delX + p_elec / deltav);
	end for;
  for j in 2:M-1 loop
		der (T[1,j]) = omega * ((T[2,j] - 2 * T[1,j] + T0[j]) / delR + (T[2,j]-T0[j]) / (r * 2 * delR) + (T[1,j+1] - 2 * T[1,j] + T[1,j-1]) / delX + p_elec / deltav);
    der (T[N,j]) = omega * ((TL[j] - 2 * T[N,j] + T[N-1,j]) / delR + (TL[j]-T[N-1,j]) / (r * 2 * delR) + (T[N,j+1] - 2 * T[N,j] + T[N,j-1]) / delX + p_elec / deltav);
  end for;
	for i in 2:N-1, j in 2:M-1 loop
		der (T[i,j]) = omega * ((T[i+1,j] - 2 * T[i,j] + T[i-1,j]) / delR + (T[i+1,j]-T[i-1,j]) / (r * 2 * delR) + (T[i,j+1] - 2 * T[i,j] + T[i,j-1]) / delX + p_elec / deltav);
  end for;
	for j in 1:M loop
    T0[M] = 4 / 3 * T[1,M] - T[2,M];
	  dTdrR[j] = -k1 * (TL[j] ^ 4 - Troom ^ 4) - k2 * (TL[j] - Troom);
	  dTdrR[j] = (-3 * TL[j] - 4*T[N,j] - T[N-1,j]) / (2*delR);
  end for;
end ELECTRICALLY_HEATED_ROD_MULTIDIM;

 
