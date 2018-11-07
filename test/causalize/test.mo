model RLC
  constant Integer N = 10;
  Real A[N];
equation
	for i in 1:N loop
		A[i] = i;
	end for;
end RLC;
