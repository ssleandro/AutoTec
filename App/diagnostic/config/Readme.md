# Add here any pertinent information concerning the module.

## STARTUP SEQUENCE:

Diagnostic module must be started BEFORE the broker, otherwise it will not be able to log broker faults.

When the broker terminates initialization, it need to inform diagnostic so it can procees with subscription