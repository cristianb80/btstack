#!/usr/bin/env python

import sys
import uuid


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("passare il file gatt seguito da quello con le callback")
        sys.exit(1)
    else:
        let = set()
        scr = set()
        ntf = set()
        lid = {}

        # Estraggo le caratteristiche e gli uuid
        with open(sys.argv[1], 'rt') as gatt:
            for riga in gatt:
                campi = riga.split(',')
                if campi[0] != 'CHARACTERISTIC':
                    pass
                else:
                    try:
                        id = campi[1].strip().upper()
                        _ = uuid.UUID(id)
                        if id in lid:
                            lid[id] += 1
                        else:
                            lid[id] = 1
                        crth = id + '-' + ('%02d' % lid[id])
                        crth = crth.replace('-', '_')
                        if 'READ' in campi[2]:
                            let.add(crth)
                        if 'WRITE' in campi[2]:
                            scr.add(crth)
                        if 'NOTIFY' in campi[2]:
                            ntf.add(crth)
                    except ValueError as e:
                        pass

        # Creo il file con lo scheletro
        with open(sys.argv[2], 'wt') as c:
            lv = let.union(scr, ntf)

            c.write('// I miei tipi\n')
            for crt in lv:
                c.write('typedef int typeof_%s ;\n' % crt)
            c.write('\n')

            c.write('// Le mie variabili\n')
            for crt in lv:
                c.write('static typeof_? val_? ;\n'.replace('?', crt))
            c.write('\n')

            if len(ntf):
                c.write('// Le mie notifiche\n')
                for crt in ntf:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_CLIENT_CONFIGURATION_HANDLE'
                    c.write('static bool ntf_%s = false ;\n' % crth)
                c.write('\n')

            if len(let):
                c.write('// Area di lettura (ipotesi: una lettura alla volta)\n')
                c.write('union {\n')
                c.write('\tuint8_t b[1] ;\n')
                for crt in let:
                    c.write('\ttypeof_? val_? ;\n'.replace('?', crt))
                c.write('} al ;\n')
                c.write('\n')

            if len(scr):
                c.write('// Area di scrittura (ipotesi: una scrittura alla volta)\n')
                c.write('union {\n')
                c.write('\tuint8_t b[1] ;\n')
                for crt in scr:
                    c.write('\ttypeof_? val_? ;\n'.replace('?', crt))
                c.write('} as ;\n')
                c.write('\n')

            if len(let):
                c.write('uint16_t le_read(uint16_t crt, uint16_t ofs, uint8_t * dati, uint16_t dim)\n')
                c.write('{\n')
                c.write('\tuint16_t letti = 0 ;\n')
                c.write('\n')
                c.write('\tif (NULL == dati) {\n')
                c.write('\t\t// Restituisco la dimensione di ogni caratteristica\n')
                c.write('\t\tswitch (crt) {\n')
                for crt in let:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_VALUE_HANDLE'
                    c.write('\t\tcase %s:\n' % crth)
                    c.write('\t\t\tletti = sizeof(typeof_%s) ; \n' % crt)
                    c.write('\t\t\tbreak ;\n')
                c.write('\t\tdefault:\n')
                c.write('\t\t\tassert(false) ;\n')
                c.write('\t\t\tbreak ;\n')
                c.write('\t\t}\n')
                c.write('\t}\n')
                c.write('\telse {\n')
                c.write('\t\t// Copio (almeno una parte del)la caratteristica\n')
                c.write('\t\tswitch(crt) {\n')
                for crt in let:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_VALUE_HANDLE'
                    c.write('\t\tcase %s:\n' % crth)
                    c.write('\t\t\tif (0 == ofs) {\n')
                    c.write('\t\t\t\t// Non thread-safe!\n')
                    c.write('\t\t\t\tal.val_? = val_? ;\n'.replace('?', crt))
                    c.write('\t\t\t}\n')
                    c.write('\t\t\tmemcpy(dati, al.b + ofs, dim) ;\n')
                    c.write('\t\t\tletti = dim ;\n')
                    c.write('\t\t\tbreak ;\n')
                c.write('\t\tdefault:\n')
                c.write('\t\t\tassert(false) ;\n')
                c.write('\t\t\tbreak ;\n')
                c.write('\t\t}\n')
                c.write('\t}\n')
                c.write('\n')
                c.write('\treturn letti ;\n')
                c.write('}\n')
                c.write('\n')

            if len(scr) + len(ntf):
                c.write('int le_write(uint16_t crt, uint16_t ofs, uint8_t * dati, uint16_t dim)\n')
                c.write('{\n')
                c.write('\tuint16_t scritti = 0 ;\n')
                c.write('\n')
                c.write('\tif (NULL == dati) {\n')
                if len(scr):
                    c.write('\t\tswitch (crt) {\n')
                    for crt in scr:
                        crth = 'ATT_CHARACTERISTIC_' + crt + '_VALUE_HANDLE'
                        c.write('\t\tcase %s:\n' % crth)
                        c.write('\t\t\tscritti = sizeof(typeof_%s) ; \n' % crt)
                        c.write('\t\t\tbreak ;\n')
                c.write('\t\tdefault:\n')
                c.write('\t\t\tassert(false) ; \n')
                c.write('\t\t\tbreak ;\n')
                c.write('\t\t}\n')
                c.write('\t}\n')
                c.write('\telse {\n')
                c.write('\t\tswitch (crt) {\n')
                for crt in ntf:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_CLIENT_CONFIGURATION_HANDLE'
                    c.write('\t\tcase %s:\n' % crth)
                    c.write('\t\t\tntf_%s = BTP_le_notify_abil(dati) ;\n' % crt)
                    c.write('\t\t\tscritti = dim ;\n')
                    c.write('\t\t\tbreak ;\n')
                for crt in scr:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_VALUE_HANDLE'
                    c.write('\t\tcase %s:\n' % crth)
                    c.write('\t\t\tmemcpy(as.b + ofs, dati, dim) ;\n')
                    c.write('\t\t\tif (ofs + dim == sizeof(typeof_%s)) {\n' % crt)
                    c.write('\t\t\t\t// Non thread-safe!\n')
                    c.write('\t\t\t\tval_? = as.val_? ;\n'.replace('?', crt))
                    c.write('\t\t\t}\n')
                    c.write('\t\t\tscritti = dim ;\n')
                    c.write('\t\t\tbreak ;\n')
                c.write('\t\tdefault:\n')
                c.write('\t\t\tassert(false) ; \n')
                c.write('\t\t\tbreak ;\n')
                c.write('\t\t}\n')
                c.write('\t}\n')
                c.write('\n')
                c.write('\treturn scritti ;\n')
                c.write('}\n')
