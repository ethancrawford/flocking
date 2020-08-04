Flock : UGen {
    *ar { |bufnum = 0, freq|
        ^this.multiNew('audio', bufnum, freq);
    }
    checkInputs {
        [1].do { |i|
            (inputs[i].rate != 'audio').if {
                ^"input % is not audio rate".format(i).throw;
            };
        };
        ^this.checkValidInputs;
    }
}