model RLC
  constant Integer N = 3;
  Real A[1],B[1];
equation
	B[1] = 3;
	A[1] * A[1] + B[1] = 9;
end RLC;
