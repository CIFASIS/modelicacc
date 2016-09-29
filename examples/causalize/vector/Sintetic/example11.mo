model A
	Real a[13,11];
equation
  for i in 1:13, j in 1:11 loop
    a[i,j]+ 2*a[i,j]=3;
  end for;
end A;
// Testing use of the same variable
