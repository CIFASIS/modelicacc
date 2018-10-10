model RLC
  constant Integer N = 10;
  Real A[N], B[N];
equation
	
	for i in 1:N loop
		A[i] + B[i] = 7;
	end for;
	for i in 1:N-1 loop
		A[i+1] - B[i] = 1;
	end for;
	A[1] = 4;
end RLC;
