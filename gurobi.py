import gurobipy

INPUT_FILE = "./gurobi_in"
THRESHILD_FILE = "long_term_threshold"

def read_threshold():
    thresholds = {}
    with open(THRESHILD_FILE, 'r') as f:
        data = f.readlines()
    for line in data:
        line = line.split()
        thresholds[int(line[0])] = int(line[1])
    return thresholds
def do_model(num_cores, num_flows, FR):
    model = gurobipy.Model("SampleModel")
    thresholds = read_threshold()
    
    # %%
    
    C = [0]*num_cores
    Tc = 10000
#    FC = [[0 for _ in range(num_flows)] for _ in range(num_cores)]
#    AC = [0]*num_cores 
    
    # %%
    MAP_g = model.addVars(num_cores * num_flows, vtype=gurobipy.GRB.BINARY, name = "MAP")
    #AC_g = model.addVars(num_cores, vtype=gurobipy.GRB.BINARY, name = "AC")
    
    
    # %%
    # All active cores should have atleast 1 flow.
    #model.addConstr(AC_g.sum()>=1, "eq2")
    #model.addConstr(sum(FC[x]) for c in range(num_cores))
    
    # Each flow should be assigned only to 1 core.
    for f in range(num_flows):
        s = gurobipy.LinExpr()
        for c in range(num_cores):
            s += MAP_g[c*num_flows + f]
        model.addConstr(s == 1, "flow_"+str(f))

    # Eq (2) Tr
    Tr_g = []
    for c in range(num_cores):
        Tr_g.append(model.addVar(name="Tr"))
        s = gurobipy.LinExpr()
        for f in range(num_flows):
            s += MAP_g[c*num_flows + f] * FR[f]
        model.addConstr(s==Tr_g[c], "Tr_set"+str(c))

    # Eq (4) 
    for c in range(num_cores):
        model.addConstr(Tr_g[c] < thresholds[Tc], "Tr_constr"+str(c))

    # Optimization function
    rate_diff = model.addVar("name=objective")
    for c in range(num_cores):
        rate_diff += thresholds[Tc] - Tr_g[c]
    model.setObjective(rate_diff, gurobipy.GRB.MINIMIZE)



    # add optimization function
    '''
    latency_variance_g = model.addVar(name="latency_variance")
    for c in range(num_cores):
        p50_estimated_g = model.addVar(name="p50_estimated_"+str(c))
        s = gurobipy.LinExpr()
        s_var = model.addVar(name="s_var_"+str(c))
        for f in range(num_flows):
            s += FC_g[c*num_flows + f] * FR[f]
        #print(s)
        #s /= 1000
        s -= 174080
        model.addConstr(s_var == s, "flow_rate"+str(c))
        model.addGenConstrLog(p50_estimated_g, s_var, "flow_rate_to_p50_"+str(c))
        model.addConstr((p50_estimated_g)<=p50_actual,"p50_estimated"+str(c))
#        model.addConstr((10**-222350*p50_estimated_g)<=p50_actual,"p50_estimated"+str(c))
#        model.addConstr((5.7*10**7*p50_estimated_g)-7.6*10**8<=p50_actual,"p50_estimated")
        latency_variance_g += p50_actual - p50_estimated_g
    model.setObjective(latency_variance_g, gurobipy.GRB.MINIMIZE)
    '''
    model.write("mips.lp")
    # %%
    model.optimize()
    
    feasible = False
    with open("gurobi_out",'a') as f:
        if model.status == gurobipy.GRB.Status.OPTIMAL:
            feasible = True
            f.write("Feasible\n")
            print(model.objVal)
        else:
            model.computeIIS()
            model.write("fail.ilp")
            model.IISConstr
            model.IISGenConstr
            f.write("Infeasible\n")
        if feasible:
            var = model.getVars()
            print(var)
            for i in range(num_cores):
                for j in range(num_flows):
                    f.write("%u"%(int(FC_g[i*num_flows + j].x)))
                f.write("\n")

def do_magic():
    while 1:
        num_cores = 0
        num_flows = 0
        FR = []
        input_file = open(INPUT_FILE, 'r')
        def read_input():
            data = input_file.read()
            if len(data) == 0:
                return
            lines = data.split('\n')
            num_cores = int(lines[0])
            num_flows = int(lines[1])
            for i in range(num_flows):
                FR.append(lines[3+i])
            do_model(num_cores,num_flows, F)
        read_input()
do_magic()
#print(read_threshold())