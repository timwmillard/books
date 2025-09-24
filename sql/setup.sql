
insert into account
    (id, type, name, normal_balance, parent_id)
values
    -- ASSETS
    (1000, 'asset', 'Current Assets', 'debit', null),
    (1010, 'asset', 'Checking Account', 'debit', 1000),
    (1020, 'asset', 'Savings Account', 'debit', 1000),
    (1030, 'asset', 'Accounts Receivable', 'debit', 1000),
    (1040, 'asset', 'Prepaid Expenses', 'debit', 1000),
    (1100, 'asset', 'Fixed Assets', 'debit', null),
    (1110, 'asset', 'Computer Equipment', 'debit', 1100),
    (1120, 'asset', 'Software & Licenses', 'debit', 1100),
    (1130, 'asset', 'Office Equipment', 'debit', 1100),

    -- LIABILITIES
    (2000, 'liability', 'Current Liabilities', 'credit', null),
    (2010, 'liability', 'Accounts Payable', 'credit', 2000),
    (2020, 'liability', 'Credit Card', 'credit', 2000),
    (2030, 'liability', 'Accrued Expenses', 'credit', 2000),
    (2040, 'liability', 'Sales Tax Payable', 'credit', 2000),
    (2100, 'liability', 'Long-term Liabilities', 'credit', null),
    (2110, 'liability', 'Equipment Loan', 'credit', 2100),

    -- EQUITY
    (3000, 'equity', 'Owner''s Equity', 'credit', null),
    (3010, 'equity', 'Owner''s Capital', 'credit', 3000),
    (3020, 'equity', 'Retained Earnings', 'credit', 3000),

    -- REVENUE
    (4000, 'revenue', 'Operating Revenue', 'credit', null),
    (4010, 'revenue', 'Software Development', 'credit', 4000),
    (4020, 'revenue', 'Consulting Services', 'credit', 4000),
    (4030, 'revenue', 'Maintenance & Support', 'credit', 4000),
    (4040, 'revenue', 'Training Services', 'credit', 4000),

    -- EXPENSES
    (5000, 'expense', 'Operating Expenses', 'credit', null),
    (5010, 'expense', 'Software & Subscriptions', 'debit', 5000),
    (5020, 'expense', 'Computer & Equipment', 'debit', 5000),
    (5030, 'expense', 'Internet & Phone', 'debit', 5000),
    (5040, 'expense', 'Office Supplies', 'debit', 5000),
    (5050, 'expense', 'Professional Services', 'debit', 5000),
    (5060, 'expense', 'Marketing & Advertising', 'debit', 5000),
    (5070, 'expense', 'Travel & Meals', 'debit', 5000),
    (5080, 'expense', 'Insurance', 'debit', 5000),
    (5090, 'expense', 'Bank Fees', 'debit', 5000),
    (5100, 'expense', 'Depreciation', 'debit', 5000);

