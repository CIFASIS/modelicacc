model Airconds
  function rand
    input Real i;
    output Real o;
  algorithm
    o := i / 2;
  end rand;

  connector ThermalConnector
    Real temperature;
  end ThermalConnector;

  connector PowerConnector
    Real power;
  end PowerConnector;

  model AC
    parameter Real Pot(fixed = false);
    discrete Real TRef(start = 20);
    ThermalConnector th_in;
    PowerConnector pow_out;
    discrete Real on;
  initial algorithm
    Pot := 13 + rand(0.2);
  equation
    pow_out.power = on * Pot;
  algorithm
    when th_in.temperature - TRef + on - 0.5 > 0 then
      on := 1;
    elsewhen th_in.temperature - TRef + on - 0.5 < 0 then
      on := 0;
    end when;
    when time > 1000 then
      TRef := 20.5;
    end when;
    when time > 2000 then
      TRef := 20;
    end when;
  end AC;

  model Room
    parameter Real Cap(fixed = false);
    parameter Real Res(fixed = false);
    parameter Real THA = 32;
    ThermalConnector th_out;
    PowerConnector ac_pow;
    Real temp(start = 20);
    discrete Real noise;
  initial algorithm
    Cap := 550 + rand(100);
    Res := 1.8 + rand(0.4);
  equation
    th_out.temperature = temp;
    der(temp) * Cap = THA / Res - ac_pow.power - temp / Res + noise / Res;
  algorithm
    when sample(0, 1) then
      noise := rand(2) - 1;
    end when;
  end Room;

  model CoolRoom
    Room room;
    AC ac;
  equation
    connect(ac.th_in, room.th_out);
    connect(ac.pow_out, room.ac_pow);
  end CoolRoom;

  constant Integer N = 10;
  CoolRoom rooms[N];
  Real Power;
equation
  Power = sum(rooms.ac.pow_out.power);
end Airconds;