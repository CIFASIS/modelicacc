model ForExample
  Integer a[10];
equation
  for i in 1:10 loop
  a[i+1] = i+1;

  end for;
end ForExample;

