model test1
	constant Integer N = 4;
	Real a[N], x;
equation
	1 + x = a[N];
	a[N] = sin(x, cos(a[1]));
	for i in 1:N-1 loop
		a[2*i + 5] + a[1]  + 1 + 4 =  a[2*i];
	end for;
end test1;
