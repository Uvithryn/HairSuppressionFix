Hides hair when slots 31 (Hair) and 41 (Long Hair) are occupied by worn item(s). 
Skyrim is supposed to do this on its own, but only looks at each armor item's first loaded ARMA record.
Many wigs are built with one ARMA in slot 31 and another in slot 41, so without this fix, the game only 
hides the main hair part, but not the LongHair part. With this fix, the whole hair model is hidden.

Also carries over Exit-9B's Beard Mask Fix, which hides beards when wearing items with slot 44.

I would have kept them separate, but both use the same hook.


## Requirements
* [ClibDT - CommonLib Dev Toolkit](**https://cmake.org/**)
	* Built with this tool
