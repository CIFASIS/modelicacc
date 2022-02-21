model matching6
  //Model with possibly wrong initial matchingTest
  constant Integer N=10000;
  Real a[N], x;
equation 
  der(x) = 1 - a[1];
  a[N] = x;
  for i in 2:N loop
    a[i - 1] = a[i];
  end for;
end test6;
