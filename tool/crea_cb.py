#!/usr/bin/env python

import sys
import uuid


def trova_uuid(prm):
    idcrt = prm.strip().upper()
    valido = False
    try:
        # Provo con quelli a 16 byte
        _ = uuid.UUID(idcrt)
        valido = True
    except ValueError as e:
        try:
            # Potrebbe essere a 4 byte
            _ = int(idcrt, 16)
            valido = True
        except ValueError as e:
            pass

    return idcrt if valido else None


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("passare il file gatt seguito da quello con le callback")
        sys.exit(1)
    else:
        # Identificativi:
            # lettura
        let = set()
            # scrittura
        scr = set()
            # notifica
        ntf = set()
            # indicazione
        ind = set()
        # Associa l'id al numero di presenze
        lid = {}
        # Associa l'id alle variabili: tipo, nome, dichiarazione
        varr = {}
        # Associa l'id alle notifiche
        varn = {}

        # Estraggo le caratteristiche e gli uuid
        with open(sys.argv[1], 'rt') as gatt:
            for riga in gatt:
                campi = riga.split(',')
                if campi[0] == 'ALIAS':
                    # ALIAS, uuid, dichiarazione (p.e. int pippo)
                    idcrt = trova_uuid(campi[1])
                    if idcrt is None:
                        print('Errore: uuid non valido in ' + riga)
                    else:
                        dich = campi[2].split()
                        if len(dich) < 2:
                            print('Errore: dichiarazione in ' + riga)
                        else:
                            nome = dich[len(dich) - 1]
                            tipo = campi[2].replace(nome, '').strip()

                            # L'alias deve essere messo (subito) dopo la descrizione della caratteristica
                            try:
                                crth = idcrt + '-' + ('%02d' % lid[idcrt])
                                crth = crth.replace('-', '_')

                                varr[crth] = (tipo, nome, '')
                            except KeyError:
                                pass

                if campi[0] == 'CHARACTERISTIC':
                    idcrt = trova_uuid(campi[1])
                    if idcrt is None:
                        print('Errore: uuid non valido in ' + riga)
                    else:
                        if idcrt in lid:
                            lid[idcrt] += 1
                        else:
                            lid[idcrt] = 1
                        # Questo viene usato da btstack
                        crth = idcrt + '-' + ('%02d' % lid[idcrt])
                        crth = crth.replace('-', '_')
                        conta = 0
                        if 'DYNAMIC' in campi[2]:
                            if 'READ' in campi[2]:
                                let.add(crth)
                                conta += 1
                            if 'WRITE' in campi[2]:
                                scr.add(crth)
                                conta += 1
                            if conta:
                                tipo = 'tip_' + crth
                                nome = 'var_' + crth
                                dich = 'typedef int ' + tipo
                                varr[crth] = (tipo, nome, dich)
                            if 'NOTIFY' in campi[2]:
                                ntf.add(crth)
                            if 'INDICATE' in campi[2]:
                                ind.add(crth)
                        else:
                            # Le caratteristiche costanti non vengono passate
                            # alle callback
                            pass

        # Notify e indication potrebbero essere attivi contemporaneamente ma:
        # Matthias Ringwald: An iOS app can only call a "subscribe" function, it cannot pick which variant
        ni = ntf.intersection(ind)
        if len(ni):
            raise Exception('ERRORE: trovate caratteristiche con notifica e indicazione')

        # Creo il file con lo scheletro
        with open(sys.argv[2], 'wt') as c:
            uls = let.union(let, scr)

            if len(uls):
                tipi = []
                for crt in uls:
                    dic = varr[crt]
                    if len(dic[2]):
                        tipi.append(dic[2] + ' ;\n')

                if len(tipi):
                    c.write('// I miei tipi\n')
                    for x in tipi:
                        c.write(x)
                    c.write('\n')

                c.write('// Le mie variabili\n')
                for crt in uls:
                    dic = varr[crt]
                    c.write('static ' + dic[0] + ' ' + dic[1] + ' ;\n')
                c.write('\n')

            if len(ntf) + len(ind):
                c.write('// Le mie notifiche o indicazioni\n')
                c.write('#define MAX_LE_NOTIF ' + str(len(ntf) + len(ind)))
                c.write('\n')
                for crt in ntf:
                    try:
                        c.write('static bool ntf_%s = false ;\n' % varr[crt][1])
                    except:
                        c.write('static bool ntf_%s = false ;\n' % crt)
                for crt in ind:
                    try:
                        c.write('static bool ind_%s = false ;\n' % varr[crt][1])
                    except:
                        c.write('static bool ind_%s = false ;\n' % crt)
                c.write('\n')

            if len(let):
                c.write(
                    '// Sala di lettura (gli accessi sono atomici in aria)\n')
                c.write('static union {\n')
                c.write('\tuint8_t b[1] ;\n')
                for crt in let:
                    dic = varr[crt]
                    c.write('\t' + dic[0] + ' ' + dic[1] + ' ;\n')
                c.write('} sdl ;\n')
                c.write('\n')

            if len(uls):
                c.write('// Gli accessi non sono (sempre) atomici nel codice\n')
                c.write('#define MUX_ACQ\n')
                c.write('#define MUX_LIB\n')
                c.write('\n')

            if len(let):
                c.write('uint16_t le_dim(uint16_t crt)\n')
                c.write('{\n')
                c.write('\tuint16_t dim = 0 ;\n')
                c.write('\n')
                c.write('\t// Restituisco la dimensione di ogni caratteristica\n')
                c.write('\tswitch (crt) {\n')
                for crt in let:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_VALUE_HANDLE'
                    dic = varr[crt]

                    c.write('\tcase %s:\n' % crth)
                    c.write('\t\tdim = sizeof(%s) ; \n' % dic[1])
                    c.write('\t\tbreak ;\n')
                for crt in ntf:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_CLIENT_CONFIGURATION_HANDLE'

                    c.write('\tcase %s:\n' % crth)
                    c.write('\t\tbreak ;\n')
                for crt in ind:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_CLIENT_CONFIGURATION_HANDLE'

                    c.write('\tcase %s:\n' % crth)
                    c.write('\t\tbreak ;\n')
                c.write('\tdefault:\n')
                c.write('\t\tassert(false) ;\n')
                c.write('\t\tbreak ;\n')
                c.write('\t}\n')
                c.write('\n')
                c.write('\treturn dim ;\n')
                c.write('}\n')
                c.write('\n')

                c.write(
                    'uint16_t le_read(uint16_t crt, uint16_t ofs, uint8_t * dati, uint16_t dim)\n')
                c.write('{\n')
                c.write('\tuint16_t letti = 0 ;\n')
                c.write('\n')
                c.write('\t// Copio (almeno una parte del)la caratteristica\n')
                c.write('\tswitch(crt) {\n')
                for crt in let:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_VALUE_HANDLE'
                    dic = varr[crt]

                    c.write('\tcase %s:\n' % crth)
                    c.write('\t\tif (0 == ofs) {\n')
                    c.write('\t\t\tMUX_ACQ ;\n')
                    c.write('\t\t\tsdl.? = ? ;\n'.replace('?', dic[1]))
                    c.write('\t\t\tMUX_LIB ;\n')
                    c.write('\t\t}\n')
                    c.write('\t\tif (dim + ofs > sizeof(%s)) {\n' % dic[1])
                    c.write('\t\t\tif (ofs < sizeof(%s))\n' % dic[1])
                    c.write('\t\t\t\tdim = sizeof(%s) - ofs ;\n' % dic[1])
                    c.write('\t\t\telse\n')
                    c.write('\t\t\t\tdim = 0 ;\n')
                    c.write('\t\t}\n')
                    c.write('\t\tletti = dim ;\n')
                    c.write('\t\tbreak ;\n')
                for crt in ntf:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_CLIENT_CONFIGURATION_HANDLE'

                    c.write('\tcase %s:\n' % crth)
                    c.write('\t\tbreak ;\n')
                for crt in ind:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_CLIENT_CONFIGURATION_HANDLE'

                    c.write('\tcase %s:\n' % crth)
                    c.write('\t\tbreak ;\n')
                c.write('\tdefault:\n')
                c.write('\t\tassert(false) ;\n')
                c.write('\t\tbreak ;\n')
                c.write('\t}\n')
                c.write('\n')
                c.write('\tif (letti)\n')
                c.write('\t\tmemcpy(dati, sdl.b + ofs, dim) ;\n')
                c.write('\n')
                c.write('\treturn letti ;\n')
                c.write('}\n')
                c.write('\n')

            if len(scr) + len(ntf) + len(ind):
                c.write('bool le_write(uint16_t crt, const uint8_t * dati, uint16_t dim)\n')
                c.write('{\n')
                c.write('\tbool esito = true ;\n')
                c.write('\n')
                c.write('\tswitch (crt) {\n')
                for crt in ntf:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_CLIENT_CONFIGURATION_HANDLE'

                    c.write('\tcase %s:\n' % crth)
                    try:
                        c.write('\t\tntf_%s = BTP_le_notify_abil(dati) ;\n' % varr[crt][1])
                    except:
                        c.write('\t\tntf_%s = BTP_le_notify_abil(dati) ;\n' % crt)
                    c.write('\t\tbreak ;\n')
                for crt in ind:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_CLIENT_CONFIGURATION_HANDLE'

                    c.write('\tcase %s:\n' % crth)
                    try:
                        c.write('\t\tind_%s = BTP_le_indicate_abil(dati) ;\n' % varr[crt][1])
                    except:
                        c.write('\t\tind_%s = BTP_le_indicate_abil(dati) ;\n' % crt)
                    c.write('\t\tbreak ;\n')
                for crt in scr:
                    crth = 'ATT_CHARACTERISTIC_' + crt + '_VALUE_HANDLE'
                    dic = varr[crt]

                    c.write('\tcase %s:\n' % crth)
                    c.write('\t\tesito = dim == sizeof(%s) ;\n' % dic[1])
                    c.write('\t\tif (esito) {\n')
                    c.write('\t\t\tMUX_ACQ ;\n')
                    c.write('\t\t\tmemcpy(& %s, dati, dim) ;\n' % dic[1])
                    c.write('\t\t\tMUX_LIB ;\n')
                    c.write('\t\t}\n')
                    c.write('\t\tbreak ;\n')
                c.write('\tdefault:\n')
                c.write('\t\tassert(false) ; \n')
                c.write('\t\tesito = false ;\n')
                c.write('\t\tbreak ;\n')
                c.write('\t}\n')
                c.write('\n')
                c.write('\treturn esito ;\n')
                c.write('}\n')
