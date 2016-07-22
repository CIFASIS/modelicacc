model test2
	constant Integer N = 3;
	Real a[N];
equation
  der(a[1])+der(a[2])+der(a[3])=0;
  der(a[2])+der(a[3])=0;
  der(a[2])+der(a[3])=0;
end test2;
