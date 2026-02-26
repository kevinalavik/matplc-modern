



//////////////////////////////////////////////////////////////////////
//
// OBSOLETE STUFF FOLLOWS - CONVERT TO FUNCTION.CPP
//
//////////////////////////////////////////////////////////////////////



int core::instructions_data(int _type){
	int	error,
		_word,
		i, start, length, start2,
		value, value2, value3, position;
	float	sourceA, sourceB, total, total2, total3;
	variable	*var1,
			*var2,
			*var3,
			*var4,
			*var5,
			*var6;

	error = NO_ERROR;



	if(_type == IL_FRD){
		pull(&value);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			// Put conversion here
			error_log("ERROR: TOD not implemented yet");
			error = set_float_in_memory(sourceA);
		}
	} else if(_type == IL_TOD){
		pull(&value);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			if((sourceA >= 0) || (sourceA <= 9999)){
			// Put conversion here
			error_log("ERROR: TOD not implemented yet");
			error = set_float_in_memory(sourceA);
			} else {
				error_log("ERROR: Too large for BCD");
			}
		}
	} else if(_type == IL_DEG){
		pull(&value);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			error = set_float_in_memory(sourceA*180/PI);
		}
	} else if(_type == IL_RAD){
		pull(&value);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			error = set_float_in_memory(sourceA*PI/180);
		}
	} else if(_type == IL_SRT){
		pull(&value);
		if(estack[stack_pointer].ladder_flag == 1) push(value);

		var1 = vstack->pop();	// array
		var2 = vstack->pop();	// status
		var3 = vstack->pop();	// start
		var4 = vstack->pop();	// length
		var5 = vstack->pop();	// position

		length = var4->var->get_int();
		start = var3->var->get_int();

		if(first_scan == TRUE){
			// mem->memory_set(_file3, _word3, _POSITION, position);
			var5->var->set_int(0);
		}
		if(value != 0){
			// mem->memory_get(_file3, _word3, _EN, &value);
			value = var2->var->get_bit(_EN);
			if(value == 0){
				var2->var->set_bit(_EN, 1);
				sourceA = (var1->var->get_variable(start))->var->get_real();
				// mem->memory_set(_file3, _word3, _EN, 1);
				// mem->memory_get(_file, _word, &sourceA);
				for(i = start+1; i < (start+length); i++){
					sourceB = (var1->var->get_variable(i))->var->get_real();
					// mem->memory_get(_file, i, &sourceB);
					if(sourceA < sourceB){
						sourceA = sourceB;
					} else if(sourceA > sourceB){
						(var1->var->get_variable(i-1))->var->set_real(sourceB);
						// mem->memory_set(_file, i-1, sourceB);
						(var1->var->get_variable(i))->var->set_real(sourceA);
						// mem->memory_set(_file, i, sourceA);
					}
				}
				sourceA = (var1->var->get_variable(start+length-1))->var->get_real();
				// mem->memory_get(_file, _word+length-1, &sourceA);
				for(i = start+length-2; i >= start; i--){
					sourceB = (var1->var->get_variable(i))->var->get_real();
					// mem->memory_get(_file, i, &sourceB);
					if(sourceA > sourceB){
						sourceA = sourceB;
					} else if(sourceA < sourceB){
						(var1->var->get_variable(i+1))->var->set_real(sourceB);
						(var1->var->get_variable(i))->var->set_real(sourceA);
						// mem->memory_set(_file, i+1, sourceB);
						// mem->memory_set(_file, i, sourceA);
					}
				}
				var2->var->set_bit(_DN, 1);
				
				// mem->memory_set(_file3, _word3, _DN, 1);
				// mem->memory_set(_file2, _word2, total/(float)length); // what did this do?
			}
		} else {
			var2->var->set_bit(_EN, 0);
			// mem->memory_set(_file3, _word3, _EN, 0);
		}
	} else if(_type == IL_AVE){
		pull(&value);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		var1 = vstack->pop();	// array
		var2 = vstack->pop();	// status
		var3 = vstack->pop();	// start
		var4 = vstack->pop();	// length
		var5 = vstack->pop();	// position
		var6 = vstack->pop();	// result
		
		start = var3->var->get_int();
		length = var4->var->get_int();
		position = var5->var->get_int();
		if(first_scan == TRUE){
			// var2->var->set_bit(_POSITION, 0);
			// mem->memory_set(_file3, _word3, _POSITION, position);
		}
		if(value != 0){
			value = var2->var->get_bit(_EN);
			// mem->memory_get(_file3, _word3, _EN, &value);
			if(value == 0){
				var2->var->set_bit(_EN, 1);
				// mem->memory_set(_file3, _word3, _EN, 1);
				total = 0.0;
				for(i = start; i < start + length; i++){
					total += (var1->var->get_variable(i))->var->get_real();
					// mem->memory_get(_file, i, &sourceA);
					// total += sourceA;
				}
				var2->var->set_bit(_EN, 1);
				// mem->memory_set(_file3, _word3, _DN, 1);
				var6->var->set_real(total/(double) length);
				// mem->memory_set(_file2, _word2, total/(float)length);
			}
		} else {
			var2->var->set_bit(_EN, 0);
			// mem->memory_set(_file3, _word3, _EN, 0);
		}
	} else if(_type == IL_STD){
		pull(&value);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		var1 = vstack->pop();	// array
		var2 = vstack->pop();	// status
		var3 = vstack->pop();	// start
		var4 = vstack->pop();	// length
		var5 = vstack->pop();	// position
		var6 = vstack->pop();	// result
		
		start = var3->var->get_int();
		length = var4->var->get_int();
		position = var5->var->get_int();
		if(first_scan == TRUE){
			// var2->var->set_bit(_POSITION, 0);
			// mem->memory_set(_file3, _word3, _POSITION, position);
		}
		if(value != 0){
			value = var2->var->get_bit(_EN);
			// mem->memory_get(_file3, _word3, _EN, &value);
			if(value == 0){
				var2->var->set_bit(_EN, 1);
				// mem->memory_set(_file3, _word3, _EN, 1);
				total = 0.0;
				for(i = start; i < start + length; i++){
					total += (var1->var->get_variable(i))->var->get_real();
					// mem->memory_get(_file, i, &sourceA);
					// total += sourceA;
				}
				total = total / (float)length;
				total2 = 0.0;
				for(i = _word; i < _word+length; i++){
					sourceA = (var1->var->get_variable(i))->var->get_real();
					// mem->memory_get(_file, i, &sourceA);
					total3 = (sourceA - total);
					total2 += total3 * total3;
				}
				total2 = total2 / (float)(length-1);
				total = sqrt(total2);
				var2->var->set_bit(_EN, 1);
				// mem->memory_set(_file3, _word3, _DN, 1);
				var6->var->set_real(total/(double) length);
				// mem->memory_set(_file2, _word2, total);
			}
		} else {
			var2->var->set_bit(_EN, 0);
			// mem->memory_set(_file3, _word3, _EN, 0);
		}


	} else if(_type == IL_MEQ){
//printf("MEQ\n");
		pull(&value);
		if(value != 0){
			var1 = vstack->pop();
			var2 = vstack->pop();
			var3 = vstack->pop();
			value = var1->var->get_int();
			value2 = var2->var->get_int();
			value3 = var3->var->get_int();
			if((value & value2) == (value2 & value3)){
				value = 1;
			}
		}
		push(value);

	} else if(_type == IL_MVM){
//printf("MVM\n");
		pull(&value2);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		if(value2 != 0){
			var1 = vstack->pop();
			var2 = vstack->pop();
			var3 = vstack->pop();
			value = var1->var->get_int();
			value2 = var2->var->get_int();
			value3 = var3->var->get_int();
			value = (value & value2) | (value3 & ~value2);
			var3->var->set_int(value);
		}
	} else if(_type == IL_FLL){
//printf("FLL\n");
		pull(&value2);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		if(value2 != 0){
			var1 = vstack->pop(); // array
			var2 = vstack->pop(); // start
			var3 = vstack->pop(); // langth
			var4 = vstack->pop(); // value
			sourceA = var4->var->get_real();
			start = var2->var->get_int();
			length = var3->var->get_int();
			// error=prog->get_operand(stack_exec[stack_pointer].prog_file,&_file,&_word,&_bit);
			// sourceB = var3->var->get_real();
			for(i = start; i < length+start; i++){
				(var1->var->get_variable(i))->var->set_real(sourceA);
				// error=mem->memory_set(_file2, _word2+i, _bit, sourceA);
			}		
		}
	} else if(_type == IL_COP){
//printf("COP\n");
		pull(&value2);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		if(value2 != 0){
			var1 = vstack->pop(); // source
			var2 = vstack->pop(); // start
			var3 = vstack->pop(); // langth
			var4 = vstack->pop(); // destination
			var5 = vstack->pop(); // destination start

			start = var2->var->get_int();
			length = var3->var->get_int();
			start2 = var5->var->get_int();

			for(i = 0; i < length; i++){
				(var4->var->get_variable(start2 + i))->var->set_real(
					(var1->var->get_variable(start+i))->var->get_real());
				// error=mem->memory_get(_file, _word+i, _bit, &value2);
				// error=mem->memory_set(_file2, _word2+i, _bit, value2);
			}		
		}
	} else {
		error = ERROR;
		error_log("ERROR: instruction type not found");
	}

	return error;
}




int core::instructions_program_control(int _type){
	int	error,
		value, value2;
	//int	value3;
	variable	*var1;

	error = NO_ERROR;
	if(_type == IL_TND){
//printf("TND\n");
		error = pull(&value);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0) estack[stack_pointer].end_flag = 1;
	} else if(_type == IL_JSR){
//printf("JSR\n");
		error = pull(&value);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			var1 = vstack->pop();
			if(error == NO_ERROR){
				stack_pointer++;
				// This needs to be replaced with a hunt for labels
				// estack[stack_pointer].prog_file = var1->var->get_int();
				//estack[stack_pointer].instruction = -1;
				estack[stack_pointer].end_flag = 0;
				estack[stack_pointer].ladder_flag = 0;
			} else {
				error_log("ERROR: can't find program number");
			}
		}
	} else if(_type == IL_RET){
//printf("RET\n");
		error = pull(&value);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			estack[stack_pointer].end_flag = 1;
		}
	} else if(_type == IL_ONS){
//printf("ONS\n");
		error = pull(&value);
		var1 = vstack->pop();
		if(value == 0){
			var1->var->set_bit(0);
		} else {
			value2 = var1->var->get_bit();
			if(value2 == 0){
				value = 1;
				var1->var->set_bit(1);
			}
		}
		push(value);
	} else if(_type == IL_JMP){
//printf("JMP\n");
		value2 = (int)get_float_from_memory();
		//if(prog->label(stack_exec[stack_pointer].prog_file, LABEL_GET_VALUE, value2, 0, &value3) == NO_ERROR){
		//	stack_exec[stack_pointer].instruction = value3;
		//} else {
		//	error_log("ERROR: Jump to label failed");
		//}
	} else if(_type == IL_LBL){
//printf("LBL\n");
		var1 = vstack->pop();
		// do nothing here
	} else {
		error = ERROR;
		error_log("ERROR: instruction type not found");
	}

	return error;
}


int core::instructions_fancy(int _type){
	int	error;
	int	int_val,
		i,
		start, length,
		_en, _ul, _dn, _er,
		number, slot, state,
		val, val2,
		mask, position;
	float	value;
	variable	*var1,
			*var2,
			*var3,
			*var4,
			*var5,
			*var6,
			*var7;
	char	temp[200];

	error = NO_ERROR;
	if(_type == IL_BSL){
//printf("BSL\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = vstack->pop();
		var2 = vstack->pop();
		var3 = vstack->pop();
		value = get_float_from_memory();
		_en = var2->var->get_bit(_EN);
		if(first_scan == TRUE){
			_en = 0; _ul = 0;
		}
		if((int_val != 0) && (_en == 0)){
			val = var3->var->get_bit(_EN);
error_log("ERROR: Not fully implemented yet - need to look at K&R");
		}
		var2->var->set_bit(_EN, int_val);
		var2->var->set_bit(_UL, int_val);
	} else if(_type == IL_BSR){
//printf("BSR\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = vstack->pop();
		var2 = vstack->pop();
		var3 = vstack->pop();
		value = get_float_from_memory();
		_en = var1->var->get_bit(_EN);
		if(first_scan == TRUE){
			_en = 0; _ul = 0;
		}
		if((int_val != 0) && (_en == 0)){
			val = var3->var->get_bit();
error_log("ERROR: Not fully implemented yet - I need to look at K&R");
		}
		var2->var->set_bit(_EN, int_val);
		var2->var->set_bit(_EU, int_val);
	} else if((_type == IL_FFL) && (_type == IL_LFL)){
//printf("FFL and LFL\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = vstack->pop(); // stack array
		var2 = vstack->pop(); // stack start
		var3 = vstack->pop(); // destination,source
		var4 = vstack->pop(); // status
		var5 = vstack->pop(); // length
		var6 = vstack->pop(); // position

		// value = get_float_from_memory();
		// value2 = get_float_from_memory();
		start = var2->var->get_int();
		_en = var4->var->get_bit(_EN);
		_dn = var4->var->get_bit(_DN);
		length = var5->var->get_int();
		position = var6->var->get_int();

		// error = mem->memory_get(_file, _word, _bit, &val);
		// error = mem->memory_get_bit(_file3, _word3, _EN, &_en);
		// error = mem->memory_get_bit(_file3, _word3, _DN, &_dn);
		// error = mem->memory_get(_file3, _word3, _POSITION, &position);
		// error = mem->memory_get(_file3, _word3, _LENGTH, &length);
		if(first_scan == TRUE){
			_en = 0; _dn = 0;
			length = (int)value;
			// error = mem->memory_set(_file2, _word2, _LENGTH, length);
			// position = (int)position;
			// if(position >= length) _dn = 1;
		}
		if((int_val != 0) && (_en == 0) && (_dn == 0)){
			position++;
			if(position >= length) _dn = 1;
			(var1->var->get_variable(start+position))->var->set_real(var2->var->get_real());
			// error = mem->memory_set(_file2, _word2+position, _bit2, val);
		}
		_en = int_val;
		var4->var->set_bit(_EN, int_val);
		var4->var->set_bit(_UL, int_val);
		var6->var->set_int(position);

		//error = mem->memory_set_bit(_file2, _word2, _EN, int_val);
		//error = mem->memory_set_bit(_file2, _word2, _UL, int_val);
		//error = mem->memory_set(_file2, _word2, _POSITION, position);
	} else if(_type == IL_FFU){
//printf("FFU\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = vstack->pop(); // stack array
		var2 = vstack->pop(); // stack start
		var3 = vstack->pop(); // destination,source
		var4 = vstack->pop(); // status
		var5 = vstack->pop(); // length
		var6 = vstack->pop(); // position
		_en = var4->var->get_bit(_EN);
		_dn = var4->var->get_bit(_DN);
		position = var6->var->get_int();
		length = var5->var->get_int();

		if(first_scan == TRUE){
			_en = 0; _dn = 0;
			// length = (int)value;
			// error = mem->memory_set(_file2, _word2, _LENGTH, length);
			// position = (int)value2;
			if(position >= length) _dn = 1;
		}
		if((int_val != 0) && (_en == 0) && (position >= 0)){
			var3->var->set_real((var1->var->get_variable(start))->var->get_real());
			// error = mem->memory_set(_file2, _word2, val);
			for(i = start; i < start+position; i++){
				(var1->var->get_variable(i))->var->set_real((var1->var->get_variable(i+1))->var->get_real());
				// mem->memory_get(_file, _word+i, _bit, &val);
				// mem->memory_set(_file, _word+i-1, _bit, val);
			}
			position--;
			if(position < 0) position = 0;
			if(position < length) _dn = 0;
		}
		_en = int_val;
		
		var4->var->set_bit(_EN, int_val);
		var4->var->set_bit(_UL, int_val);
		var6->var->set_int(position);
		// error = mem->memory_set_bit(_file2, _word2, _EN, int_val);
		// error = mem->memory_set_bit(_file2, _word2, _UL, int_val);
		// error = mem->memory_set(_file2, _word2, _POSITION, position);
	} else if(_type == IL_LFU){
//printf("LFU\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = vstack->pop(); // stack array
		var2 = vstack->pop(); // stack start
		var3 = vstack->pop(); // destination,source
		var4 = vstack->pop(); // status
		var5 = vstack->pop(); // length
		var6 = vstack->pop(); // position

		_en = var4->var->get_bit(_EN);
		_dn = var4->var->get_bit(_DN);
		start = var2->var->get_int();
		length = var5->var->get_int();
		position = var6->var->get_int();

		if(first_scan == TRUE){
			_en = 0; _dn = 0;
			// length = (int)value;
			// error = mem->memory_set(_file2, _word2, _LENGTH, length);
			// position = (int)value2;
			if(position >= length) _dn = 1;
		}
		if((int_val != 0) && (_en == 0) && (position >= 0)){
			var3->var->set_real((var1->var->get_variable(start+position))->var->get_real());
			position--;
			if(position < 0) position = 0;
			if(position < length) _dn = 0;
		}
		_en = int_val;
		var4->var->set_bit(_EN, _en);
		var4->var->set_bit(_DN, _dn);
		var6->var->set_int(position);
	} else if(_type == IL_SQO){
//printf("SQO\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = vstack->pop(); // sequencer array
		var2 = vstack->pop(); // sequencer start
		var3 = vstack->pop(); // destination,source
		var4 = vstack->pop(); // status
		var5 = vstack->pop(); // length
		var6 = vstack->pop(); // position
		var7 = vstack->pop(); // mask

		_en = var4->var->get_bit(_EN);
		start = var2->var->get_int();
		length = var5->var->get_int();
		position = var6->var->get_int();
		mask = var7->var->get_int();

		if(first_scan == TRUE){
			_en = 0;
			// length = (int)value;
			// error = mem->memory_set(_file2, _word2, _LENGTH, length);
			// position = (int)value2;
			if(position >= length) position = 0;
		}
		if((int_val != 0) && (_en == 0)){
			position++;
			if(position > length) position = 1;
			val = (var1->var->get_variable(start+position))->var->get_int();
			val2 = var3->var->get_int();
			var3->var->set_int((val & mask) + (val2 & ~mask));
			// error = mem->memory_get(_file, _word+position, _bit, &val);
			// error = mem->memory_set(_file2, _word2, _bit2, val);
		}
		_en = int_val;
		var4->var->set_bit(_EN, _en);
		var6->var->set_int(position);
	} else if(_type == IL_SQI){
printf("SQI - not implemented now\n");
	} else if(_type == IL_SQL){
printf("SQL - not implemented now\n");

	} else if(_type == IL_EML){
//printf("EML\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = vstack->pop(); // To:
		var2 = vstack->pop(); // Subject
		var3 = vstack->pop(); // message
		var4 = vstack->pop(); // control
		var5 = vstack->pop(); // slot
		_en = var4->var->get_bit(_EN);
		_dn = var4->var->get_bit(_DN);
		_er = var4->var->get_bit(_ER);

		if(first_scan == TRUE){
			_en = 0; _dn = 0; _er = 0;
		}
		if((int_val != 0) && (_en == 0)){
			sprintf(temp, "EMAIL %s \"%s\" \"%s\"", var1->var->get_string(), var2->var->get_string(), var3->var->get_string());
			number = communications->add(temp);
			_en = 1; _dn = 0; _er = 0;
			communications->status(number, _WAITING);
			var5->var->set_int(number);
		} else if((_en != 0) && (_dn == 0)){
			slot = var5->var->get_int();
			state = communications->status(slot);
			if(state == _DONE){
				_dn = 1;
				communications->release(slot);
			} else if(state == _ERROR){
				_dn = 1; _er = 1;
				communications->release(slot);
			}
		}
		if(_dn == 1) _en = int_val;
		var4->var->set_bit(_EN, _en);
		var4->var->set_bit(_DN, _dn);
		var4->var->set_bit(_ER, _er);
	} else {
		error = ERROR;
		error_log("ERROR: Fancy functions not recognized");
	}

	return error;
}




int core::instructions_ascii(int _type){
	int	error;
	int	int_val,
		length, start,
		_en, _dn, _er,
//		dest,
		number, slot, state;
	char	text1[100];
	char	*texttemp;
	float	value;
	variable	*var1, *var2, *var3, *var4, *var5;

	error = NO_ERROR;
	if(_type == IL_AIC){
//printf("AIC\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		if(int_val != 0){
			var1 = vstack->pop(); // integer source
			var2 = vstack->pop(); // string destination
			var2->var->set_string(var1->var->get_string());
		}
	} else if(_type == IL_ACI){
//printf("ACI\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		if(int_val != 0){
			var1 = vstack->pop(); // string source
			var2 = vstack->pop(); // number destination
			var2->var->set_real(var1->var->get_real());
		}
	} else if(_type == IL_ASC){
//printf("ASC\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
error_log("WARNING: Not implemented - I wasn't sure about this - no manuals at the time");
		if(int_val != 0){
		}
	} else if(_type == IL_ACN){
//printf("ACN\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		if(int_val != 0){
			var1 = vstack->pop(); // string base
			var2 = vstack->pop(); // appended string
			texttemp = new char[strlen(var1->var->get_string())+strlen(var2->var->get_string())+1];
			strcpy(texttemp, var1->var->get_string());
			strcat(texttemp, var2->var->get_string());
			var1->var->set_string(texttemp);
		}
	} else if(_type == IL_ASR){
//printf("ASR\n");
		pull(&int_val);
		if(int_val != 0){
			var1 = vstack->pop(); // string A
			var2 = vstack->pop(); // string B
			if(strcmp(var1->var->get_string(), var2->var->get_string()) != 0) int_val = 0;
		}
		push(int_val);
	} else if(_type == IL_AEX){
//printf("AEX\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		if(int_val != 0){
			var1 = vstack->pop(); // source string
			var2 = vstack->pop(); // start
			var3 = vstack->pop(); // length
			var4 = vstack->pop(); // destination string

			texttemp = new char[length+1];
			strncpy(texttemp, &(var1->var->get_string()[start]), length);
			var4->var->set_string(texttemp);
		}
	} else if(_type == IL_ARL){
//printf("ARL\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = vstack->pop(); // channel
		var2 = vstack->pop(); // destination
		var3 = vstack->pop(); // status
		var4 = vstack->pop(); // length
		var5 = vstack->pop(); // position

		value = var1->var->get_int();
		_en = var3->var->get_bit(_EN);
		_dn = var3->var->get_bit(_DN);
		_er = var3->var->get_bit(_ER);

		if(first_scan == TRUE){
			_en = 0; _dn = 0; _er = 0;
		}
		if((int_val != 0) && (_en == 0)){
			if((int)value == 0){ 
				sprintf(text1, "APPEND COM1 SEND MEMORY %s", var2->var->get_symbol());
			} else if((int)value == 1){
				sprintf(text1, "APPEND COM2 SEND MEMORY %s", var2->var->get_symbol());
			} else {
				error_log("WARNING: Channel number was incorrect using channel 0");
				sprintf(text1, "APPEND COM1 SEND MEMORY %s", var2->var->get_symbol());
			}
			number = communications->add(text1);
			_en = 1; _dn = 0; _er = 0;
			communications->status(number, _WAITING);
			var5->var->set_int(number);
			// error = mem->memory_set(_file2, _word2, _POSITION, number);
		} else if((_en != 0) && (_dn == 0)){
			slot = var5->var->get_int();
			// error = mem->memory_get(_file2, _word2, _POSITION, &slot);
			state = communications->status(slot);
			if(state == _DONE){
				_dn = 1;
				communications->release(slot);
			} else if(state == _ERROR){
				_dn = 1; _er = 1;
				communications->release(slot);
			}
		}
		if(_dn == 1){
//printf("soup 5 \n");
			_en = int_val;
		}
		var3->var->set_bit(_EN, _en);
		var3->var->set_bit(_DN, _dn);
		var3->var->set_bit(_ER, _er);
	} else if(_type == IL_AWT){
//printf("AWT\n");
		pull(&int_val);
		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = vstack->pop(); // channel
		var2 = vstack->pop(); // source
		var3 = vstack->pop(); // status
		var4 = vstack->pop(); // length
		var5 = vstack->pop(); // position

		value = var1->var->get_int();
		length = var4->var->get_int();
		_en = var3->var->get_bit(_EN);
		_dn = var3->var->get_bit(_DN);
		_er = var3->var->get_bit(_ER);

		if(first_scan == TRUE){
			_en = 0; _dn = 0; _er = 0;
		}
		if((int_val != 0) && (_en == 0)){
			if((int)value == 0){
				sprintf(text1, "APPEND MEMORY %s SEND COM1", var2->var->get_symbol());
			} else if((int)value == 1){
				sprintf(text1, "APPEND MEMORY %s SEND COM2", var2->var->get_symbol());
			} else {
				error_log("WARNING: Channel number was incorrect using channel 0");
				sprintf(text1, "APPEND MEMORY %s SEND COM1", var2->var->get_symbol());
			}
			number = communications->add(text1);
			_en = 1; _dn = 0; _er = 0;
			communications->status(number, _WAITING);
			var5->var->set_int(number);
			// error = mem->memory_set(_file2, _word2, _POSITION, number);
		} else if((_en != 0) && (_dn == 0)){
			slot = var5->var->get_int();
			// error = mem->memory_get(_file2, _word2, _POSITION, &slot);
			state = communications->status(slot);
			if(state == _DONE){
				_dn = 1;
				communications->release(slot);
			} else if(state == _ERROR){
				_dn = 1; _er = 1;
				communications->release(slot);
			}
		}
		if(_dn == 1) _en = int_val;
		var3->var->set_bit(_EN, _en);
		var3->var->set_bit(_DN, _dn);
		var3->var->set_bit(_ER, _er);
	} else {
		error_log("ERROR: ASCII Function not recognized");
		error = ERROR;
	}

	return error;
}






float core::get_float_from_memory(){
	variable	*var1;

	var1 = vstack->pop();

	return var1->var->get_real();
}


int core::set_float_in_memory(float value){
	int	temp;
	variable	*var1;

	var1 = vstack->pop();
	var1->var->set_real(value);

	return temp;
}
	


int core::push(int value){
	int	error;

	error = ERROR;
	if(stack.count < STACK_SIZE){
		stack.count++;
		stack.list[stack.count] = value;
		error = NO_ERROR;
	} else {
		error_log("too many arguments on stack");
	}

	return error;
}


int core::pull(int *value){
	int	error;

	error = ERROR;
	if(stack.count >=0){
		value[0] = stack.list[stack.count];
		stack.count--;
		error = NO_ERROR;
	} else {
		value[0] = 1;
		error_log("stack is empty");
	}
	return error;
}






int core::instructions_timer_counter(int _type){
	int	error,
		value,
		// time_diff,
		_en, _dn, _tt, _cu, _cd, _ov, _un;
	double	_preset, _accumulator;
		//, _time_base;
	variable	*var1, *var2, *var3, *var4;

	error = NO_ERROR;



	} else if((_type == IL_CTU) || (_type == IL_CTD)){
//printf("CTU and CTD\n");
		var1 = vstack->pop(); // status
		var2 = vstack->pop(); // limit
		var3 = vstack->pop(); // accumulator

		// error =prog->get_operand(stack_exec[stack_pointer].prog_file,&_file,&_word,&_bit);
		_preset = var2->var->get_int();
		_accumulator = var3->var->get_int();
		_cu = var1->var->get_bit(_CU);
		_cd = var1->var->get_bit(_CD);
		_dn = var1->var->get_bit(_DN);
		_ov = var1->var->get_bit(_OV);
		_un = var1->var->get_bit(_UN);
		if(first_scan == TRUE){
			// mem->memory_set(_file, _word, _ACCUMULATOR, (int)_accumulator);
			// mem->memory_set(_file, _word, _PRESET, (int)_preset);
		} else {
			// mem->memory_get(_file, _word, _PRESET, &value);
			// _preset = (float)value;
			// mem->memory_get(_file,_word,_ACCUMULATOR, &value);
			// _accumulator = (float)value;
		}

		pull(&value);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		if((_type == IL_CTU) && (value != 0) && (_cu == 0)){
			_cu = 1;
			if(_accumulator >= 32767){
				_ov = 1;
				_accumulator = -32768;
				error_log("ERROR: counter overflow");
				_dn = 0;
			} else {
				_accumulator++;
				if(_accumulator >= _preset){
					_dn = 1;
				} else {
					_dn = 0;
				}
			}
		} else if((_type == IL_CTD) && (value != 0) &&(_cd==0)){
			_cd = 1;
			if(_accumulator <= -32768){
				_un = 1;
				_accumulator = 32767;
				error_log("ERROR: counter underflow");
				_dn = 0;
			} else {
				_accumulator--;
				if(_accumulator <= _preset){
					_dn = 1;
				} else {
					_dn = 0;
				}
			}
		} else if(value == 0) {
			_cu = 0;
			_cd = 0;
		}
		var1->var->set_bit(_CU, _cu);
		var1->var->set_bit(_CD, _cd);
		var1->var->set_bit(_DN, _dn);
		var1->var->set_bit(_OV, _ov);
		var1->var->set_bit(_UN, _un);
		var3->var->set_real(_accumulator);

		// mem->memory_set(_file, _word, _ACCUMULATOR, (int)_accumulator);
	} else if(_type == IL_RES){
//printf("RES\n");
		var1 = vstack->pop(); // status
		var2 = vstack->pop(); // accumulator
		// error =prog->get_operand(stack_exec[stack_pointer].prog_file,&_file,&_word,&_bit);
		// mem->memory_info(_file, &_used, &type_temp, &_size);
		pull(&value);
		if(estack[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			var1->var->set_int(0);
			var2->var->set_real(0.0);
		}
	} else {
		error = ERROR;
		error_log("ERROR: instruction type not found");
	}

	return error;
}




