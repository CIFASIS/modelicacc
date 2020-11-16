model RLC
  constant Integer N = 10;
  Real C, A[N], B[N];
equation
	C + A[3] + B[3] = 9;
	for i in 1:N loop
		A[i] + B[i] = 7;
		A[i] - B[i] = 1;
	end for;
end RLC;
