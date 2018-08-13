model RLC
  constant Integer N = 1000000000;
  Real A[N], B[N];
equation
	for i in 1:1 loop
		A[1000000000] = 5;
	end for;
	for i in 1:N loop
		A[i] + B[i] = 12;
	end for;
	for i in 2:N loop
		B[i-1] = 8;
	end for;
end RLC;
