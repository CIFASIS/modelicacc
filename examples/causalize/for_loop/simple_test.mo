model simple_test
	constant Integer N = 800;
	constant Integer M = 20;
	Real a[N];
	Real b[6], x;
	parameter Real R1;
equation
  b[4] = 5;
	b[2] = 4;
	b[6] = 9;
	for i in 1:3 loop
		b[2*i - 1] = 4;
	end for;
	for i in 1:N/2 loop
		a[2*i] + R1 = 0;
	end for;
	for i in 1:N/2 loop
		a[2*i-1] + x = 0;
	end for;
	x = 9;
end simple_test;
