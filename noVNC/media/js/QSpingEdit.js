function QSpinEdit(EditId, minValue, maxValue)
{
	//properties
	this.EditId = EditId;//deprecated
	this.ObjectRef = [];
	this.ObjectId = [];
	this.ObjectId['Edit'] = EditId;
	this.ObjectId['UpArrow'] = "up"+EditId;
	this.ObjectId['DownArrow'] = "down"+EditId;
	this.DefaultValue = minValue;
	this.MinValue = minValue;
	this.MaxValue = maxValue;
	this.Size = 2; //no effect after WriteControl
	this.BackgroundColor = 'white';
	this.Style = 'vert'; //no effect after WriteControl
	this.ReadOnly = false; //read-only property
	this.TimerObj = null;
	this.Step = -1;

	//public methods
	QSpinEdit.prototype.AddSubStep = AddSubStep;
	QSpinEdit.prototype.WriteControl = WriteControl;
	QSpinEdit.prototype.GetValue = GetValue;
	QSpinEdit.prototype.SetValue = SetValue;
	QSpinEdit.prototype.Validate = Validate;
	QSpinEdit.prototype.Disable = Disable;
	QSpinEdit.prototype.Enable = Enable;
	QSpinEdit.prototype.ResetValue = ResetValue;


	//internal event handlers; there is not need to call them directly
	QSpinEdit.prototype.HandleChange = HandleChange;

	//private function members
	var FixJSFloatBug = function FixJSFloatBug(n){return Math.round(n*100)/100;}
	
	//events
	QSpinEdit.prototype.OnChange = function OnChange(){};
	QSpinEdit.prototype.OnValidateError = function OnValidateError(){};

	//-----------------------------------------------------------------
	function Validate(theValue)
	{
		if (theValue === '' || theValue == null || theValue > this.MaxValue || theValue < this.MinValue || isFinite(theValue) == 0 ||  String(FixJSFloatBug(theValue)).indexOf('.') !=- 1) {
			this.OnValidateError();
			return false;
		} else {
			//prevent bug when user enters '07' for example in the edit field
			this.ObjectRef['Edit'].value = FixJSFloatBug(theValue*1); //little trick to convert to numeric and prevent JS Floating point operation bugs
			return true;
		}
	}
	
	function AddSubStep(mode)
	{
		if (this.TimerObj) {
			clearTimeout(this.TimerObj);
		}
		
		if (this.ObjectRef['Edit'].disabled) {
			return;
		}
		
		var OldValue = this.n;
		if (this.Step > 0) {
			this.n = FixJSFloatBug((mode == "add") ? this.n+this.Step : this.n-this.Step);
			if (this.n < this.MinValue) {
				this.n = this.MinValue;
			} else if (this.n > this.MaxValue) {
				this.n = this.MaxValue;
			}
		} else {
			var i = 0;
			for (i=0; i<=5; i++) {
				var tmp = Math.pow(2, i)*10; //10,20,40,80,160
				if (this.n == tmp) {
					this.n = FixJSFloatBug((mode == "add") ? tmp*2 : tmp/2);
					if (this.n < 10) {
						this.n = FixJSFloatBug(10);
					} else if (this.n > 320) {
						this.n = FixJSFloatBug(320);
					}
					break;
				} else if (this.n < tmp) {
					if ((tmp == 10) && (mode == "sub")) {
						this.n = FixJSFloatBug(this.MinValue);
					} else {
						this.n = FixJSFloatBug((mode == "add") ? tmp : tmp/2);
					}
					break;
				}
			}
		}
		
		if (this.n != OldValue) {
			this.ObjectRef['Edit'].value = this.n;
			this.OnChange();
		}
		
		var  myObj = this;
		this.TimerObj = setTimeout(function(){myObj.AddSubStep(mode)}, 200);
	}
	
	function WriteControl(SpinName)
	{
		this.n = this.DefaultValue;
		var readonly_str = '';
		if (this.ReadOnly) {
			readonly_str = 'readonly="readonly"';
		}
		
		document.write('<table border="0" summary="" cellspacing="0" cellpadding="0" style="display:inline;vertical-align:middle">');
		document.write('<tr>');
	
		if(this.Style == "vert") {
			document.write('<td rowspan="2"><input style="width:100%;height:24px;" size="'+this.Size+'" id="'+this.ObjectId['Edit']+'" name="'+this.ObjectId['Edit']+'" type="text" value="'+this.DefaultValue+'" onchange="'+SpinName+'.HandleChange(false)" onkeyup="'+SpinName+'.HandleChange(true)" '+readonly_str+' /></td>');
			document.write('<td width="50px"><div class="_bullet_arrow_up" style="margin:-5px 0 0 10px;"><input type="button" style="font-size:8px;color:black;width:16px;height:14px;vertical-align:bottom;padding:0px;" id="'+this.ObjectId['UpArrow']+'" onmousedown="this.blur();'+SpinName+'.AddSubStep(\'add\')" onmouseup="clearTimeout('+SpinName+'.TimerObj);" /></div></td>');
			document.write('</tr>');
			document.write('<tr>');
			document.write('<td width="50px"><div class="_bullet_arrow_down" style="margin:-3px 0 0 10px;"><input type="button" style="font-size:8px;color:black;width:16px;height:14px;vertical-align:top;padding:0px;" id="'+this.ObjectId['DownArrow']+'" onmousedown="this.blur();'+SpinName+'.AddSubStep(\'sub\')" onmouseup="clearTimeout('+SpinName+'.TimerObj);" /></div></td>');
		} else {	//horizontal
			document.write('<td rowspan="2"><input style="width:100%;height:24px;" <input size="'+this.Size+'" id="'+this.ObjectId['Edit']+'" name="'+this.ObjectId['Edit']+'" type="text" value="'+this.DefaultValue+'"  onchange="'+SpinName+'.HandleChange(false)" onkeyup="'+SpinName+'.HandleChange(true)" '+readonly_str+' /></td>');
			document.write('<td width="50px"><div class="_bullet_arrow_up" style="margin:-5px 0 0 10px;"><input type="button" style="font-size:11px;color:black;width:18px;height:24px;padding:0px;" id="'+this.ObjectId['UpArrow']+'" onmousedown="this.blur();'+SpinName+'.AddSubStep(\'add\')" onmouseup="clearTimeout('+SpinName+'.TimerObj);" /></div></td>');
			document.write('<td width="50px"><div class="_bullet_arrow_down" style="margin:-5px 0 0 10px;"><input type="button" style="font-size:11px;color:black;width:18px;height:24px;padding:0px;" id="'+this.ObjectId['DownArrow']+'" onmousedown="this.blur();'+SpinName+'.AddSubStep(\'sub\')" onmouseup="clearTimeout('+SpinName+'.TimerObj);" /></div></td>');
		}
		document.write('</tr>');
		document.write('</table>');
		
		//store objects references for faster access 
		this.ObjectRef['Edit'] = document.getElementById(this.ObjectId['Edit']);
		this.ObjectRef['UpArrow'] = document.getElementById(this.ObjectId['UpArrow']);
		this.ObjectRef['DownArrow'] = document.getElementById(this.ObjectId['DownArrow']);	
	}
	
	//-----------------------------------------------------------------
	//GetValue
	function GetValue()
	{
		return this.n;
	}
	//-----------------------------------------------------------------
	//SetValue
	function SetValue(newValue)
	{
		//validation
		if (!this.Validate(newValue) || this.ObjectRef['Edit'].disabled) {
			return false;
		}
		
		this.n = newValue * 1;
		this.ObjectRef['Edit'].value = this.n;
		this.OnChange();
	}
	//-----------------------------------------------------------------
	//ResetValue
	function ResetValue()
	{
		this.n = this.DefaultValue;
		this.ObjectRef['Edit'].value = this.n;
		this.OnChange();
	}		

	//-----------------------------------------------------------------
	function HandleChange(bFromKey)
	{
		this.ObjectRef['Edit'].style.backgroundColor = this.BackgroundColor;
		if (!this.Validate(this.ObjectRef['Edit'].value)) {
			if (!bFromKey) {
				this.ObjectRef['Edit'].value = this.n;
			} else {
				this.ObjectRef['Edit'].style.backgroundColor = 'yellow';
			}
			this.ObjectRef['Edit'].focus();	//set focus warn user
			return false;
		} else {
			this.ObjectRef['Edit'].style.backgroundColor = 'white';
			this.n = FixJSFloatBug(this.ObjectRef['Edit'].value*1);	//little trick to convert to numeric and prevent JS Floating point operation bugs
		}
		
		this.OnChange();
		return true;
	}
	//-----------------------------------------------------------------
	function Disable()
	{
		this.ObjectRef['Edit'].disabled = true;
		this.ObjectRef['UpArrow'].disabled = true;
		this.ObjectRef['DownArrow'].disabled = true;
	}
	//-----------------------------------------------------------------
	function Enable()
	{
		this.ObjectRef['Edit'].disabled = false;
		this.ObjectRef['UpArrow'].disabled = false;
		this.ObjectRef['DownArrow'].disabled = false;
	}
}