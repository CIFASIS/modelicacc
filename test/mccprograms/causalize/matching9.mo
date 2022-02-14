  model matching9
    //Model with possibly wrong initial matchingTest
    constant Integer N=100000000;
    Real x1[N], a1[N], b1[N];
/*
    Real x2[N], a2[N], b2[N];
    Real x3[N], a3[N], b3[N];
    Real x4[N], a4[N], b4[N];
    Real x5[N], a5[N], b5[N];
    Real x6[N], a6[N], b6[N];
    Real x7[N], a7[N], b7[N];
    Real x8[N], a8[N], b8[N];
    Real x9[N], a9[N], b9[N];
    Real x10[N], a10[N], b10[N];
    Real x11[N], a11[N], b11[N];
    Real x12[N], a12[N], b12[N];
    Real x13[N], a13[N], b13[N];
    Real x14[N], a14[N], b14[N];
    Real x15[N], a15[N], b15[N];
*/
  equation 
    for i in 2:N-1 loop
      der(x1[i]) = a1[i] - x1[i];
      a1[i + 1] = a1[i] + b1[i];
      b1[i] = x1[i - 1];
    end for;
    b1[1] = 0;
    b1[N] = x1[N - 1];
    der(x1[1]) = a1[1] - x1[1];
    der(x1[N]) = a1[N] - x1[N];
    a1[1] = 1;
    a1[N] = a1[N - 1] + b1[N - 1];

/*
    for i in 2:N-1 loop
      der(x2[i]) = a2[i] - x2[i];
      a2[i + 1] = a2[i] + b2[i];
      b2[i] = x2[i - 1];
    end for;
    b2[1] = 0;
    b2[N] = x2[N - 1];
    der(x2[1]) = a2[1] - x2[1];
    der(x2[N]) = a2[N] - x2[N];
    a2[1] = 1;
    a2[N] = a2[N - 1] + b2[N - 1];

    for i in 2:N-1 loop
      der(x3[i]) = a3[i] - x3[i];
      a3[i + 1] = a3[i] + b3[i];
      b3[i] = x3[i - 1];
    end for;
    b3[1] = 0;
    b3[N] = x3[N - 1];
    der(x3[1]) = a3[1] - x3[1];
    der(x3[N]) = a3[N] - x3[N];
    a3[1] = 1;
    a3[N] = a3[N - 1] + b3[N - 1];

    for i in 2:N-1 loop
      der(x4[i]) = a4[i] - x4[i];
      a4[i + 1] = a4[i] + b4[i];
      b4[i] = x4[i - 1];
    end for;
    b4[1] = 0;
    b4[N] = x4[N - 1];
    der(x4[1]) = a4[1] - x4[1];
    der(x4[N]) = a4[N] - x4[N];
    a4[1] = 1;
    a4[N] = a4[N - 1] + b4[N - 1];

    for i in 2:N-1 loop
      der(x5[i]) = a5[i] - x5[i];
      a5[i + 1] = a5[i] + b5[i];
      b5[i] = x5[i - 1];
    end for;
    b5[1] = 0;
    b5[N] = x5[N - 1];
    der(x5[1]) = a5[1] - x5[1];
    der(x5[N]) = a5[N] - x5[N];
    a5[1] = 1;
    a5[N] = a5[N - 1] + b5[N - 1];

    for i in 2:N-1 loop
      der(x6[i]) = a6[i] - x6[i];
      a6[i + 1] = a6[i] + b6[i];
      b6[i] = x6[i - 1];
    end for;
    b6[1] = 0;
    b6[N] = x6[N - 1];
    der(x6[1]) = a6[1] - x6[1];
    der(x6[N]) = a6[N] - x6[N];
    a6[1] = 1;
    a6[N] = a6[N - 1] + b6[N - 1];

    for i in 2:N-1 loop
      der(x7[i]) = a7[i] - x7[i];
      a7[i + 1] = a7[i] + b7[i];
      b7[i] = x7[i - 1];
    end for;
    b7[1] = 0;
    b7[N] = x7[N - 1];
    der(x7[1]) = a7[1] - x7[1];
    der(x7[N]) = a7[N] - x7[N];
    a7[1] = 1;
    a7[N] = a7[N - 1] + b7[N - 1];

    for i in 2:N-1 loop
      der(x8[i]) = a8[i] - x8[i];
      a8[i + 1] = a8[i] + b8[i];
      b8[i] = x8[i - 1];
    end for;
    b8[1] = 0;
    b8[N] = x8[N - 1];
    der(x8[1]) = a8[1] - x8[1];
    der(x8[N]) = a8[N] - x8[N];
    a8[1] = 1;
    a8[N] = a8[N - 1] + b8[N - 1];

    for i in 2:N-1 loop
      der(x9[i]) = a9[i] - x9[i];
      a9[i + 1] = a9[i] + b9[i];
      b9[i] = x9[i - 1];
    end for;
    b9[1] = 0;
    b9[N] = x9[N - 1];
    der(x9[1]) = a9[1] - x9[1];
    der(x9[N]) = a9[N] - x9[N];
    a9[1] = 1;
    a9[N] = a9[N - 1] + b9[N - 1];

    for i in 2:N-1 loop
      der(x10[i]) = a10[i] - x10[i];
      a10[i + 1] = a10[i] + b10[i];
      b10[i] = x10[i - 1];
    end for;
    b10[1] = 0;
    b10[N] = x10[N - 1];
    der(x10[1]) = a10[1] - x10[1];
    der(x10[N]) = a10[N] - x10[N];
    a10[1] = 1;
    a10[N] = a10[N - 1] + b10[N - 1];

    for i in 2:N-1 loop
      der(x11[i]) = a11[i] - x11[i];
      a11[i + 1] = a11[i] + b11[i];
      b11[i] = x11[i - 1];
    end for;
    b11[1] = 0;
    b11[N] = x11[N - 1];
    der(x11[1]) = a11[1] - x11[1];
    der(x11[N]) = a11[N] - x11[N];
    a11[1] = 1;
    a11[N] = a11[N - 1] + b11[N - 1];

    for i in 2:N-1 loop
      der(x12[i]) = a12[i] - x12[i];
      a12[i + 1] = a12[i] + b12[i];
      b12[i] = x12[i - 1];
    end for;
    b12[1] = 0;
    b12[N] = x12[N - 1];
    der(x12[1]) = a12[1] - x12[1];
    der(x12[N]) = a12[N] - x12[N];
    a12[1] = 1;
    a12[N] = a12[N - 1] + b12[N - 1];

    for i in 2:N-1 loop
      der(x13[i]) = a13[i] - x13[i];
      a13[i + 1] = a13[i] + b13[i];
      b13[i] = x13[i - 1];
    end for;
    b13[1] = 0;
    b13[N] = x13[N - 1];
    der(x13[1]) = a13[1] - x13[1];
    der(x13[N]) = a13[N] - x13[N];
    a13[1] = 1;
    a13[N] = a13[N - 1] + b13[N - 1];

    for i in 2:N-1 loop
      der(x14[i]) = a14[i] - x14[i];
      a14[i + 1] = a14[i] + b14[i];
      b14[i] = x14[i - 1];
    end for;
    b14[1] = 0;
    b14[N] = x14[N - 1];
    der(x14[1]) = a14[1] - x14[1];
    der(x14[N]) = a14[N] - x14[N];
    a14[1] = 1;
    a14[N] = a14[N - 1] + b14[N - 1];

    for i in 2:N-1 loop
      der(x15[i]) = a15[i] - x15[i];
      a15[i + 1] = a15[i] + b15[i];
      b15[i] = x15[i - 1];
    end for;
    b15[1] = 0;
    b15[N] = x15[N - 1];
    der(x15[1]) = a15[1] - x15[1];
    der(x15[N]) = a15[N] - x15[N];
    a15[1] = 1;
    a15[N] = a15[N - 1] + b15[N - 1];
*/
  end matching9;
