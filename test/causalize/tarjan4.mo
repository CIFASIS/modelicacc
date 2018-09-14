model RLC
  constant Integer N = 3;
  Real A[N];
equation
	A[1] + A[2] = 4;
	A[2] + A[3] = 7;
	A[3] + A[1] = 15;
end RLC;
