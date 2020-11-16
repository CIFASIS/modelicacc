model RLC
  constant Integer N = 3;
  Real A[N],B[N];
equation
	B[1] = 1;
	B[2] = 2;
	B[3] = 3;
  for i in 1:N loop
	A[i] * A[i] + B[i] = 9;
	end for;
end RLC;
