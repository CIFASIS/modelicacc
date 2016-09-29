model A
  Real a[5];
equation
  for i in 1:5 loop
    a[i]+a[3]=17;
  end for;
end A;

// Remove duplicates: OK