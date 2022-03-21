model Test2
  //Model with possibly wrong initial matching
  constant Integer N = 100;
  Real a[N], x[N], b[N];
equation 
  for i in 2:N-1 loop
    der(x[i]) = a[i] - x[i];
    a[i + 1] = a[i] + b[i];
    b[i] = x[i - 1];
  end for;
  b[1] = 0;
  b[N] = x[N - 1];
  der(x[1]) = a[1] - x[1];
  der(x[N]) = a[N] - x[N];
  a[2] = a[1] + b[1];
  a[N] = 1;
end Test2;
