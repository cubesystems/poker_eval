require "./pokereval"
t = PokerEval.new       
x = t.eval({"game"=>"holdem", "pockets"=>[["Ks","Kd"],["As","Ad"]], "board"=>["Ac","Kc"], "iterations" => 10000, "fill_pockets" => 0})
print x
