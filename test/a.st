fn test(a:Event<i32> , b:State<i32>){
  c = a + stateChangeEvents(b);
  d = 2 * c;
  return d;
}

fn add(a:Event<i32> , b:Event<i32>){
  return a+b;
}

fn main(a:i64,b:i64){
  c = a + b;
  d = c * 2;
  e = d + 1;
  x = add(d,e);
  stx = latestEventToState(x);
  y = stateChangeEvents(stx);
  z = test(y,stx);
  return z;
}
