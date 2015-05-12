function QFlot(bodyObjID)
{
	this.bodyObjID = bodyObjID;
	this.DataArray = [];
	this.PreviousData = [];
	this.totalPoints = 180; // 180*5s=15min
	
	
	//public methods
	QFlot.prototype.InsertData = InsertData;
	QFlot.prototype.Repaint = Repaint;
	QFlot.prototype.GetData = GetData;
	
	 // private constructor 
	__construct = function(that) {
	}(this)
	
	function GetData(idx) {  
		var maxValue = 0;     
        var result = []; 	
		var rxData = [];
		var txData = [];
		for (var i=0; i<this.totalPoints-this.DataArray[idx].rx.length; i++) { 
			rxData.push([i, 0]);
			txData.push([i, 0]);
		}
		for (var i=0; i<this.DataArray[idx].rx.length; i++) { 
			rxData.push([rxData.length, this.DataArray[idx].rx[i]]); 
			txData.push([txData.length, this.DataArray[idx].tx[i]]);
		}
		
		var tmpMax = Math.max.apply(Math, this.DataArray[idx].rx); 
		if (maxValue < tmpMax) {
			maxValue = tmpMax;
		}
		tmpMax = Math.max.apply(Math, this.DataArray[idx].tx); 
		if (maxValue < tmpMax) {
			maxValue = tmpMax;
		}

		result.push({label: gettext('Packets received'), data: rxData});
		result.push({label: gettext('Packets sent'), data: txData});
		return {'maxValue':maxValue, 'result':result};
    }
	
	function InsertData(bridge_name, rx, tx) {
		if (this.DataArray.length == 0) {
			this.DataArray.push({'bridge_name':bridge_name, 'rx':[rx/(1024*5)], 'tx':[tx/(1024*5)]});
			return;
		}
		
		for (var i=0; i<this.DataArray.length; i++) {
			if (this.DataArray[i].bridge_name == bridge_name) {
				this.DataArray[i].rx.push(rx/(1024*5)); // kb/s
				this.DataArray[i].tx.push(tx/(1024*5)); 
			
				if (this.DataArray[i].rx.length > this.totalPoints) {
					this.DataArray[i].rx.shift();
					this.DataArray[i].tx.shift(); 
				}
				break;
			}
		}
	}
	
	function Repaint() {
		var resultData = this.GetData(0);
		var maxValue = resultData.maxValue/0.9;
		if (maxValue < 400) {
			maxValue = 400;
		}
		var myTicksX =  [];
		for (var i=0; i<=5; i++) {
			var xValue = this.totalPoints/5*i;
			if(i == 5) {
				myTicksX.push([xValue, String((this.totalPoints-xValue)/12)+' MINs']);
			}
			else {
			myTicksX.push([xValue, String((this.totalPoints-xValue)/12)]);
		}
		}
		var myTicksY =  [[0, '0KB/s']];
		for (var i=1; i<5; i++) {
			var yValue = maxValue/4*i;
			if (yValue >= 1024*1024) {
				myTicksY.push([yValue, (yValue/1024/1024).toFixed(0) + 'GB']);
			} else if (yValue >= 1024) {
				myTicksY.push([yValue, (yValue/1024).toFixed(0) + 'MB']);
			} else {
				myTicksY.push([yValue, yValue.toFixed(0) + 'KB']);
			}
		}
		
		var plot = $.plot($('#'+this.bodyObjID), 
					resultData.result,
					{ 
						colors: ["#5E4D93" , "#FCCD06"],
						legend: { 
							container:$("#legendContainer"),
							//position: 'nw', 
							labelBoxBorderColor: "#FFFFFF"
							//noColumns: 0 
						},
						xaxis: { 
							min: 0, 
							max: this.totalPoints, 
							ticks: myTicksX
						},
						yaxis: { 
							min: 0,
							max: maxValue,
							ticks: myTicksY
						},
						grid:{ backgroundColor: { colors: ["#ffffff", "#ffffff"] } } 
						
					} 
				); 
		plot.draw();
	}

}