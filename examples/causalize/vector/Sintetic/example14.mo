model A
	Real a[12];
equation
  for i in 1:2 loop
    a[i]+a[11]=123;
  end for;
  for i in 1:10 loop
    a[i]+a[12]=0;
  end for;
end A;
