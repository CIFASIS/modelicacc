model REDRC
  constant Integer N = 5;
  Real iC[N];
	Real iR[N];
	Real uR[N];
	Real uC[N];
	parameter Real iL;
	parameter Real v;
	parameter Real R[N];
	parameter Real C[N];	
equation
	for j in 1:N loop
		R[j] * iR[j] - uR[j] = 0;
		C[j] * der (uC[j]) - iC[j] = 0;
	end for;
	for i in 1:N-1 loop
		iR[i] - iC[i] - iR[i+1] = 0;
	end for;
	for i in 2:N loop
		uC[i-1] - uR[i] - uC[i] = 0;
	end for;
	v - uR[1] - uC[1] = 0;
	iR[N] - iC[N] - iL = 0;
end REDRC;
