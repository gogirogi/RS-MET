rsEllipticSubBandFilterDirectForm::rsEllipticSubBandFilterDirectForm()
{
  reset();  
  setSubDivision(2.0); 
}

void rsEllipticSubBandFilterDirectForm::setSubDivision(double newSubDivision)
{
  rsEllipticSubBandFilter tmpBiquad; // make member to avoid memory allocation
  tmpBiquad.setSubDivision(newSubDivision);
  rsFilterCoefficientConverter::biquadCascadeToDirectForm(6, tmpBiquad.getAddressB0(), 
    tmpBiquad.getAddressB1(), tmpBiquad.getAddressB2(), tmpBiquad.getAddressA1(), 
    tmpBiquad.getAddressA2(), b, a);
}

void rsEllipticSubBandFilterDirectForm::reset()
{
  for(int i = 0; i < 12; i++)
    w[i] = 0.0;
}
