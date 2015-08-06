model derivative
	constant Integer N = 200;
	Real a[N], x, y;
equation
	x = 2+der(a[2]);
	y = x;
  for i in 1:N loop
	  der(a[i]) - x*y=0;
  end for;

end derivative;
