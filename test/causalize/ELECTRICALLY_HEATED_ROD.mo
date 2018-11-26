model ELECTRICALLY_HEATED_ROD
  constant Integer N = 10;
  parameter Real delR=1, p_elec=1, deltav=1, r=1, omega=1, k1=1, k2=1, Troom=1;
	Real T[N], T0, TL, dTdrR;
equation
	der (T[1]) = omega * ((T[2] - 2 * T[1] + T0) / delR + (T[2]-T0) / (r * 2 * delR) + p_elec / deltav);
  for i in 2:N-1 loop
		der (T[i]) = omega * ((T[i+1] - 2 * T[i] + T[i-1]) / delR + (T[i+1]-T[i-1]) / (r * 2 * delR) + p_elec / deltav);
  end for;
	der (T[N]) = omega * ((TL - 2 * T[N] + T[N-1]) / delR + (TL-T[N-1]) / (r * 2 * delR) + p_elec / deltav);
  T0 = 4 / 3 * T[1] - T[2];
	dTdrR = -k1 * (TL ^ 4 - Troom ^ 4) - k2 * (TL - Troom);
	dTdrR = (-3 * TL - 4*T[N] - T[N-1]) / (2*delR);
end ELECTRICALLY_HEATED_ROD;

 
