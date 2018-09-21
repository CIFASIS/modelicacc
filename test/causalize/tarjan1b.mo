model RLC
  constant Integer N = 1;
  Real A[N], B[N];
equation
	A[1] + B[1] = 7;
	A[1] - B[1] = 1;
end RLC;
