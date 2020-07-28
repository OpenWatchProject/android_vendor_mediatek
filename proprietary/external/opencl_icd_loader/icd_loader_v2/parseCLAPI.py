def main():

	# Phase1 : parse api function prototype from "icd_dispatch.cpp" to "trace.in"
	#		   as if many line to be single line
	sFile = open( "icd_dispatch.c", "r" )
	iFile = iter(sFile)
	wFile = open( "trace.in", "w")

	countCL_API_ENTRY = 0

	#look for patterns
	for line in iFile:

		argsNum = 1

		line = line.strip("\s\t\n")
		if line.find( "CL_API_ENTRY" ) != -1:
			countCL_API_ENTRY = countCL_API_ENTRY + 1

			while True:
				line = next(iFile)
				line = line.strip("\s\t\n")

				wFile.write(line);

				if line.find( "CL_CALLBACK" ) != -1:
					continue;

				if line.find( "," ) != -1:
					argsNum = argsNum + 1

				if line.find( "(void)" ) != -1:
					argsNum = 0

				if line.find( ")" ) != -1:
					wFile.write("\n");
					wFile.write(str(argsNum));
					wFile.write("\n");
					break;

	print( "CL_API_ENTRY: ", countCL_API_ENTRY )
	sFile.close
	wFile.close

	# Before Phase2, use notepad++ to split the args with " | "

	# Phase2: parse api args and format to "clTrace.in"
	sFile = open("trace.in", "r")
	iFile = iter(sFile)
	wFile = open("clTrace.in", "w")

	for line in iFile:
		line = line.strip("\s\t\n")
		argsNum = next(iFile)
		argsNum = argsNum.strip("\s\t\n")

		apiName, rest = line.split(" | ",1)

		wFile.write(" ( ")
		wFile.write(apiName)
		wFile.write(", ")
		wFile.write(argsNum)

		if argsNum != 0:
			wFile.write(", ")

		count = int(argsNum)
		while count > 0:

			if count != 1:

				# Separate type and value pair by " | " from other pairs
				# ex int a | bool b | char c, split "int a" in arg and store others in rest
				arg, rest = rest.split(" | ",1)

				# Separate type and value by space
				type, val = arg.split(" ",1)

				# if type is const, then have to separate type and value with second space
				# ex: const char* value has to be split to "const char*" and "value"
				if type.find("const") != -1:
					secType, val = val.split(" ",1)

			# Split the last one type and value pair
			if count == 1:
				type, val = rest.split(" ",1)
				if type.find("const") != -1:
					secType, val = val.split(" ",1)

			wFile.write("\"")
			wFile.write(type)

			if type == "const":
				wFile.write(" ")
				wFile.write(secType)
			wFile.write("\", ")
			wFile.write(val)


			if count > 1:
				wFile.write(", ")

			count -= 1

		wFile.write(" ) \n")

	sFile.close()
	wFile.close()

	# Phase3 : replace CL_TRACE_CALL(...) to the new string generated in Phase2
	ricdFile = open( "icd_dispatch.cpp", "r" )
	icdFile = iter(ricdFile)
	rclFile = open( "clTrace.in", "r" )
	clFile = iter(rclFile)

	wicdFile = open( "icd_dispatch-new.cpp", "w")

	for line in ricdFile:
		if line.find("CL_TRACE_CALL") != -1:
			wicdFile.write("CL_TRACE_CALL")
			replaceLine = next(rclFile)
			print(replaceLine)
			wicdFile.writelines(replaceLine)

		else:
			wicdFile.writelines(line)


	ricdFile.close()
	rclFile.close()
	wicdFile.close()

main()
