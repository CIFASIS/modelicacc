model RLC
  constant Integer N = 1000000;
  Real Y, A[N];
equation
	A[3] = 4;
	for i in 1:N loop
		A[i] + Y = 7;
	end for;
end RLC;
