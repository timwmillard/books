-- Sample journal entries
insert into journal (date, description, reference)
    values ('2024-01-15', 'Software purchase - Adobe Creative Suite', 'INV-001'),
    ('2024-01-16', 'Client payment received', 'INV-2024-001'),
    ('2024-01-17', 'Office supplies purchase', 'REC-789'),
    ('2024-01-18', 'Internet bill payment', 'BILL-456'),
    ('2024-01-20', 'Equipment purchase - New laptop', 'REC-890');

-- Sample journal lines
insert into journal_line (journal_id, account_id, amount)
    values
    -- Adobe Creative Suite purchase (debit software, credit checking)
    (1, 1120, 50000), -- Software & Licenses +$500
    (1, 1010, -50000), -- Checking Account -$500
    -- Client payment (debit checking, credit revenue)
    (2, 1010, 150000), -- Checking Account +$1,500
    (2, 4010, -150000), -- Software Development -$1,500
    -- Office supplies (debit supplies, credit credit card)
    (3, 5040, 7500), -- Office Supplies +$75
    (3, 2020, -7500), -- Credit Card -$75
    -- Internet bill (debit internet, credit checking)
    (4, 5030, 8000), -- Internet & Phone +$80
    (4, 1010, -8000), -- Checking Account -$80
    -- Laptop purchase (debit equipment, credit equipment loan)
    (5, 1110, 200000), -- Computer Equipment +$2,000
    (5, 2110, -200000) -- Equipment Loan -$2,000
;

