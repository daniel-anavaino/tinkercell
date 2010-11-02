function A = fromTC(arg)
	try  %items array
	    n = arg.length;
		A = zeros(1,n);
		for i=1:n
		    A(i) = tinkercell.tc_getItem(arg,i-1);
		endfor
		tinkercell.tc_deleteItemsArray(arg);
    catch
	    try %strings array
		    n = arg.length;
			A = 1:n;
			for i=1:n
				A(i) = tinkercell.tc_getString(arg,i-1);
			endfor
			tinkercell.tc_deleteStringsArray(arg);
		catch
			try %matrix
				m = arg.rows;
				n = arg.cols;
				A = zeros(m,n);
				for i=1:m
					for j=1:n
						A(i,j) = tinkercell.tc_getMatrixValue(arg,i-1,j-1);
					endfor
				endfor
				tinkercell.tc_deleteMatrix(arg);
			catch
				A = arg;
			end_try_catch %matrix
		end_try_catch %strings array
	end_try_catch %items array
endfunction

