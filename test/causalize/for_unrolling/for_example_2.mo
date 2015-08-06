model ForExample
  constant Integer N = 11;
  constant Integer S = 1;
  constant Integer M = 20;
  Integer a[10];
equation
  for i in N:S:M loop
  a[i] = i;

  end for;
end ForExample;

