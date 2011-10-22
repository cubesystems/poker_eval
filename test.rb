require "./pokereval"
t = PokerEval.new       
x = t.eval({"game"=>"holdem", "pockets"=>[["Qc","Jh"],["9h","6c"],["4c","4h"],["6h","8c"]], "board"=>["Ac","Kc"],  "iterations" => 1000})
print x["info"]
print "\n"
for i in x["eval"]
  print i
  print "\n"
end
