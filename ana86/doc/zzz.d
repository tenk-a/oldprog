
	ModuleName:export far module 
		Mod2:inport ModuleName2
		VarName:WORD var
		CVarName:const BYTE(20) var = "test file"
		ConstName:const = 200
		StructName:struct 
			MembName1:WORD
			MembName2:BYTE(20)
			MembName4:type(VarName)
			end
		AryName:WORD(5,5,5)
		ProcName:proc (Arg1:WORD,Arg2:BYTE(30))
			AutoName1:WORD
			AutoName2:StructName
			Arg1 += 2
			Mod2.print (1,2,3)
			Arg1 = 2
			AutoName1 = Mod2.eee(Arg1)
			end
		ProcName2:proc
			
